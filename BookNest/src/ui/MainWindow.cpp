#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QStatusBar>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionProgressBar>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QToolBar>
#include <QVBoxLayout>

#include "BookDialog.h"
#include "EstatisticasDialog.h"
#include "GoogleBooksDialog.h"
#include "ListasDialog.h"
#include "ProgressDialog.h"
#include "ReviewDialog.h"
#include "db/DatabaseManager.h"

namespace booknest {

namespace {
// Colunas da view vw_acervo
enum Col { CId, CTitulo, CAutor, CGenero, CAno, CIsbn, CPaginas, CStatus, CSituacao, CPaginaAtual, CProgresso, CAvaliacao };

// Desenha a coluna "progresso" como barra (REU002).
class ProgressDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx) const override {
        QStyleOptionProgressBar bar;
        bar.rect = opt.rect.adjusted(4, 4, -4, -4);
        bar.minimum = 0;
        bar.maximum = 1000;
        const double v = idx.data().toDouble();
        bar.progress = static_cast<int>(v * 10);
        bar.text = QStringLiteral("%1%").arg(v, 0, 'f', 1);
        bar.textVisible = true;
        bar.state = opt.state | QStyle::State_Horizontal;
        bar.palette = opt.palette;
        if (opt.state & QStyle::State_Selected) p->fillRect(opt.rect, opt.palette.highlight());
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &bar, p);
    }
};

QString sqlStr(QString s) { return s.replace('\'', QStringLiteral("''")); }
QString likeEsc(QString s) {
    s.replace('\\', QStringLiteral("\\\\"));
    s.replace('%', QStringLiteral("\\%"));
    s.replace('_', QStringLiteral("\\_"));
    return sqlStr(s);
}
}  // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("BookNest — Sua biblioteca pessoal"));
    resize(1100, 620);
    montarInterface();
    montarAcoes();
    recarregarGeneros();
    aplicarFiltro();
}

void MainWindow::montarInterface() {
    m_model = new QSqlTableModel(this, DatabaseManager::instance().database());
    m_model->setTable(QStringLiteral("vw_acervo"));
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_model->setSort(CTitulo, Qt::AscendingOrder);
    m_model->select();  // cabeçalhos devem ser definidos depois do primeiro select()
    const QStringList cab = {QString(), tr("Título"), tr("Autor"), tr("Gênero"), tr("Ano"),
                             tr("ISBN"), tr("Páginas"), QString(), tr("Situação"), tr("Pág. atual"),
                             tr("Progresso"), tr("Avaliação")};
    for (int i = 0; i < cab.size(); ++i) m_model->setHeaderData(i, Qt::Horizontal, cab[i]);

    m_view = new QTableView;
    m_view->setModel(m_model);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setAlternatingRowColors(true);
    m_view->setSortingEnabled(true);
    m_view->sortByColumn(CTitulo, Qt::AscendingOrder);
    m_view->verticalHeader()->hide();
    m_view->hideColumn(CId);
    m_view->hideColumn(CStatus);
    m_view->setItemDelegateForColumn(CProgresso, new ProgressDelegate(m_view));
    auto* h = m_view->horizontalHeader();
    h->setSectionResizeMode(CTitulo, QHeaderView::Stretch);
    h->setSectionResizeMode(CAutor, QHeaderView::Stretch);
    m_view->setColumnWidth(CGenero, 130);
    m_view->setColumnWidth(CAno, 60);
    m_view->setColumnWidth(CIsbn, 120);
    m_view->setColumnWidth(CPaginas, 70);
    m_view->setColumnWidth(CSituacao, 90);
    m_view->setColumnWidth(CPaginaAtual, 80);
    m_view->setColumnWidth(CProgresso, 130);
    m_view->setColumnWidth(CAvaliacao, 90);
    connect(m_view, &QTableView::doubleClicked, this, [this] { editar(); });

    m_busca = new QLineEdit;
    m_busca->setPlaceholderText(tr("🔍  Buscar por título ou autor…"));
    m_busca->setClearButtonEnabled(true);
    m_filtroStatus = new QComboBox;
    m_filtroStatus->addItem(tr("Todos os status"), QString());
    m_filtroStatus->addItem(tr("Quero Ler"), QStringLiteral("QUERO_LER"));
    m_filtroStatus->addItem(tr("Lendo"), QStringLiteral("LENDO"));
    m_filtroStatus->addItem(tr("Lido"), QStringLiteral("LIDO"));
    m_filtroStatus->addItem(tr("Abandonado"), QStringLiteral("ABANDONADO"));
    m_filtroGenero = new QComboBox;
    m_contador = new QLabel;

    connect(m_busca, &QLineEdit::textChanged, this, &MainWindow::aplicarFiltro);
    connect(m_filtroStatus, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { aplicarFiltro(); });
    connect(m_filtroGenero, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { aplicarFiltro(); });

    auto* barra = new QHBoxLayout;
    barra->addWidget(m_busca, 3);
    barra->addWidget(m_filtroStatus, 1);
    barra->addWidget(m_filtroGenero, 1);

    auto* central = new QWidget;
    auto* lay = new QVBoxLayout(central);
    lay->addLayout(barra);
    lay->addWidget(m_view, 1);
    lay->addWidget(m_contador);
    setCentralWidget(central);
}

void MainWindow::montarAcoes() {
    auto* aCad = new QAction(tr("Cadastrar livro"), this);
    aCad->setShortcut(QKeySequence::New);
    auto* aEdit = new QAction(tr("Editar"), this);
    auto* aRem = new QAction(tr("Remover"), this);
    aRem->setShortcut(QKeySequence::Delete);
    auto* aProg = new QAction(tr("Atualizar progresso"), this);
    auto* aAval = new QAction(tr("Avaliar / resenha"), this);
    auto* aDet = new QAction(tr("Ver detalhes"), this);
    auto* aListas = new QAction(tr("Listas temáticas"), this);
    auto* aEst = new QAction(tr("Histórico e estatísticas"), this);
    auto* aWeb = new QAction(tr("Buscar na internet"), this);
    auto* aSair = new QAction(tr("Sair"), this);
    aSair->setShortcut(QKeySequence::Quit);

    connect(aCad, &QAction::triggered, this, &MainWindow::cadastrar);
    connect(aEdit, &QAction::triggered, this, &MainWindow::editar);
    connect(aRem, &QAction::triggered, this, &MainWindow::remover);
    connect(aProg, &QAction::triggered, this, &MainWindow::registrarProgresso);
    connect(aAval, &QAction::triggered, this, &MainWindow::avaliar);
    connect(aDet, &QAction::triggered, this, &MainWindow::verDetalhes);
    connect(aListas, &QAction::triggered, this, &MainWindow::abrirListas);
    connect(aEst, &QAction::triggered, this, &MainWindow::abrirEstatisticas);
    connect(aWeb, &QAction::triggered, this, &MainWindow::buscarNaWeb);
    connect(aSair, &QAction::triggered, this, &QWidget::close);

    auto* mLivro = menuBar()->addMenu(tr("&Livro"));
    mLivro->addActions({aCad, aEdit, aRem});
    mLivro->addSeparator();
    mLivro->addAction(aDet);
    mLivro->addSeparator();
    mLivro->addAction(aSair);

    auto* mLeit = menuBar()->addMenu(tr("L&eitura"));
    mLeit->addAction(aProg);
    mLeit->addAction(aAval);
    auto* mStatus = mLeit->addMenu(tr("Marcar como"));
    struct { const char* nome; core::Status s; } opts[] = {
        {"Quero Ler", core::Status::QueroLer}, {"Lendo", core::Status::Lendo},
        {"Lido", core::Status::Lido}, {"Abandonado", core::Status::Abandonado}};
    for (const auto& o : opts) {
        auto* a = mStatus->addAction(tr(o.nome));
        const core::Status s = o.s;
        connect(a, &QAction::triggered, this, [this, s] { alterarStatus(s); });
    }

    auto* mFerr = menuBar()->addMenu(tr("&Ferramentas"));
    mFerr->addAction(aListas);
    mFerr->addAction(aEst);
    mFerr->addAction(aWeb);

    auto* tb = addToolBar(tr("Principal"));
    tb->setMovable(false);
    tb->setToolButtonStyle(Qt::ToolButtonTextOnly);
    tb->addActions({aCad, aEdit, aRem});
    tb->addSeparator();
    tb->addActions({aProg, aAval});
    tb->addSeparator();
    tb->addActions({aListas, aEst, aWeb});
}

void MainWindow::mensagem(const QString& t) { statusBar()->showMessage(t, 5000); }

qint64 MainWindow::livroSelecionado(bool avisar) {
    const QModelIndex idx = m_view->currentIndex();
    if (!idx.isValid()) {
        if (avisar)
            QMessageBox::information(this, tr("Selecione um livro"),
                                     tr("Selecione um livro na tabela para continuar."));
        return 0;
    }
    return m_model->data(m_model->index(idx.row(), CId)).toLongLong();
}

void MainWindow::recarregarGeneros() {
    const QString atual = m_filtroGenero->currentData().toString();
    m_filtroGenero->blockSignals(true);
    m_filtroGenero->clear();
    m_filtroGenero->addItem(tr("Todos os gêneros"), QString());
    for (const auto& g : m_livros.generos()) m_filtroGenero->addItem(g, g);
    const int i = m_filtroGenero->findData(atual);
    m_filtroGenero->setCurrentIndex(i < 0 ? 0 : i);
    m_filtroGenero->blockSignals(false);
}

void MainWindow::aplicarFiltro() {
    QStringList cond;
    const QString termo = m_busca->text().trimmed();
    if (!termo.isEmpty()) {
        const QString t = likeEsc(termo);
        cond << QStringLiteral("(titulo LIKE '%%1%' ESCAPE '\\' OR autor LIKE '%%1%' ESCAPE '\\')").arg(t);
    }
    const QString st = m_filtroStatus->currentData().toString();
    if (!st.isEmpty()) cond << QStringLiteral("status = '%1'").arg(sqlStr(st));
    const QString ge = m_filtroGenero->currentData().toString();
    if (!ge.isEmpty()) cond << QStringLiteral("genero = '%1'").arg(sqlStr(ge));

    m_model->setFilter(cond.join(QStringLiteral(" AND ")));  // FA01: texto vazio remove o filtro
    m_model->select();
    while (m_model->canFetchMore()) m_model->fetchMore();
    m_contador->setText(tr("%1 livro(s) exibido(s)").arg(m_model->rowCount()));  // FE01: vazio é válido
}

void MainWindow::recarregar() {
    const qint64 id = livroSelecionado(false);
    recarregarGeneros();
    aplicarFiltro();
    if (id != 0) {
        for (int r = 0; r < m_model->rowCount(); ++r)
            if (m_model->data(m_model->index(r, CId)).toLongLong() == id) {
                m_view->selectRow(r);
                break;
            }
    }
}

void MainWindow::cadastrar() {
    BookDialog dlg(m_livros.autores(), m_livros.generos(), nullptr, this);
    if (dlg.exec() != QDialog::Accepted) return;
    QString erro;
    if (!m_livros.inserir(dlg.livro(), &erro)) {
        QMessageBox::warning(this, tr("Não foi possível cadastrar"), erro);
        return;
    }
    recarregar();
    mensagem(tr("Livro cadastrado com sucesso."));
}

void MainWindow::editar() {
    const qint64 id = livroSelecionado();
    if (id == 0) return;
    auto livro = m_livros.buscar(id);
    if (!livro) {
        QMessageBox::critical(this, tr("Erro"), tr("Falha ao ler o livro no banco de dados."));  // FE01
        return;
    }
    BookDialog dlg(m_livros.autores(), m_livros.generos(), &*livro, this);
    if (dlg.exec() != QDialog::Accepted) return;
    QString erro;
    if (!m_livros.atualizar(dlg.livro(), &erro)) {
        QMessageBox::critical(this, tr("Não foi possível atualizar"), erro);
        return;
    }
    recarregar();
    mensagem(tr("Livro atualizado."));
}

void MainWindow::remover() {
    const qint64 id = livroSelecionado();
    if (id == 0) return;
    auto livro = m_livros.buscar(id);
    const QString nome = livro ? livro->titulo : tr("o livro selecionado");
    const auto r = QMessageBox::question(
        this, tr("Confirmar exclusão"),
        tr("Deseja realmente remover \"%1\"?\nO progresso, a nota e a resenha também serão apagados.")
            .arg(nome),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);  // RN004
    if (r != QMessageBox::Yes) return;                         // FA01
    QString erro;
    if (!m_livros.remover(id, &erro)) {
        QMessageBox::warning(this, tr("Erro ao remover"), erro);
        return;
    }
    recarregar();
    mensagem(tr("Livro removido."));
}

void MainWindow::registrarProgresso() {
    const qint64 id = livroSelecionado();
    if (id == 0) return;
    auto livro = m_livros.buscar(id);
    auto leitura = m_leituras.porLivro(id);
    if (!livro || !leitura) return;

    if (leitura->status != core::Status::Lendo) {
        const auto r = QMessageBox::question(
            this, tr("Iniciar leitura"),
            tr("O progresso só pode ser registrado para livros com status \"Lendo\".\n"
               "Deseja marcar \"%1\" como \"Lendo\" agora?").arg(livro->titulo));
        if (r != QMessageBox::Yes) return;
        QString erro;
        if (!m_leituras.alterarStatus(id, core::Status::Lendo, &erro)) {
            QMessageBox::warning(this, tr("Erro"), erro);
            return;
        }
        leitura = m_leituras.porLivro(id);
    }

    ProgressDialog dlg(livro->titulo, leitura->paginaAtual, livro->totalPaginas, this);
    if (dlg.exec() != QDialog::Accepted) return;

    core::ProgressOutcome out;
    QString erro;
    if (!m_leituras.registrarProgresso(id, dlg.novaPagina(), &out, &erro)) {
        QMessageBox::warning(this, tr("Progresso inválido"), erro);  // FE01 / RN005
        return;
    }
    recarregar();
    if (out.finished)
        QMessageBox::information(this, tr("Leitura concluída"),
                                 tr("Parabéns! \"%1\" foi marcado como Lido. "
                                    "Que tal avaliar o livro?").arg(livro->titulo));
    else
        mensagem(tr("Progresso atualizado: %1%").arg(out.percent, 0, 'f', 1));
}

void MainWindow::avaliar() {
    const qint64 id = livroSelecionado();
    if (id == 0) return;
    auto livro = m_livros.buscar(id);
    auto leitura = m_leituras.porLivro(id);
    if (!livro || !leitura) return;
    const auto v = core::validateRating(leitura->status, 5);
    if (!v.ok) {
        QMessageBox::information(this, tr("Avaliação indisponível"), QString::fromStdString(v.message));
        return;
    }
    ReviewDialog dlg(livro->titulo, leitura->nota, leitura->resenha, this);
    if (dlg.exec() != QDialog::Accepted) return;
    QString erro;
    if (!m_leituras.avaliar(id, dlg.nota(), dlg.resenha(), &erro)) {
        QMessageBox::warning(this, tr("Erro"), erro);
        return;
    }
    recarregar();
    mensagem(tr("Avaliação salva."));
}

void MainWindow::alterarStatus(core::Status novo) {
    const qint64 id = livroSelecionado();
    if (id == 0) return;
    QString erro;
    if (!m_leituras.alterarStatus(id, novo, &erro)) {
        QMessageBox::warning(this, tr("Erro"), erro);
        return;
    }
    recarregar();
    mensagem(tr("Status alterado para %1.").arg(QString::fromStdString(core::statusLabel(novo))));
}

void MainWindow::verDetalhes() {
    const qint64 id = livroSelecionado();
    if (id == 0) return;
    auto l = m_livros.buscar(id);
    auto r = m_leituras.porLivro(id);
    if (!l || !r) return;
    QString html = QStringLiteral("<h3>%1</h3><p><i>%2</i> — %3").arg(
        l->titulo.toHtmlEscaped(), l->autor.toHtmlEscaped(), l->genero.toHtmlEscaped());
    if (l->anoPublicacao) html += QStringLiteral(" (%1)").arg(l->anoPublicacao);
    html += QStringLiteral("</p>");
    if (!l->sinopse.isEmpty()) html += QStringLiteral("<p>%1</p>").arg(l->sinopse.toHtmlEscaped());
    html += tr("<p><b>Situação:</b> %1 — página %2 de %3 (%4%)</p>")
                .arg(QString::fromStdString(core::statusLabel(r->status)))
                .arg(r->paginaAtual).arg(l->totalPaginas)
                .arg(core::progressPercent(r->paginaAtual, l->totalPaginas), 0, 'f', 1);
    if (r->nota > 0) html += tr("<p><b>Nota:</b> %1</p>").arg(QString(r->nota, QChar(0x2605)));
    if (!r->resenha.isEmpty())
        html += tr("<p><b>Resenha:</b><br>%1</p>").arg(r->resenha.toHtmlEscaped());
    const auto hist = m_leituras.historico(id);
    if (!hist.isEmpty()) {
        html += tr("<p><b>Últimos registros de progresso:</b><br>");
        for (int i = 0; i < qMin(5, hist.size()); ++i)
            html += tr("%1 — página %2 (+%3)<br>")
                        .arg(hist[i].data.toString(QStringLiteral("dd/MM/yyyy")))
                        .arg(hist[i].paginaAtual).arg(hist[i].paginasLidas);
        html += QStringLiteral("</p>");
    }
    QMessageBox box(this);
    box.setWindowTitle(tr("Detalhes do livro"));
    box.setTextFormat(Qt::RichText);
    box.setText(html);
    box.exec();
}

void MainWindow::abrirListas() {
    ListasDialog dlg(this);
    dlg.exec();
}

void MainWindow::abrirEstatisticas() {
    EstatisticasDialog dlg(this);
    dlg.exec();
}

void MainWindow::buscarNaWeb() {
    GoogleBooksDialog dlg(this);
    dlg.exec();
    if (dlg.adicionouLivro()) recarregar();
}

}  // namespace booknest
