#include "GoogleBooksDialog.h"
#include "core/Trace.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "BookDialog.h"

namespace booknest {

GoogleBooksDialog::GoogleBooksDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Buscar livros na internet"));
    resize(820, 480);

    m_termo = new QLineEdit;
    m_termo->setPlaceholderText(tr("Título, autor ou ISBN…"));
    m_btnBuscar = new QPushButton(tr("Buscar"));
    m_btnBuscar->setDefault(true);
    m_tabela = new QTableWidget(0, 5);
    m_tabela->setHorizontalHeaderLabels(
        {tr("Título"), tr("Autor"), tr("Gênero"), tr("Ano"), tr("Páginas")});
    m_tabela->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tabela->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tabela->verticalHeader()->hide();
    m_tabela->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tabela->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tabela->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_status = new QLabel(tr("Digite um termo e clique em Buscar. Requer conexão com a internet."));
    auto* btnAdd = new QPushButton(tr("Adicionar à biblioteca…"));
    auto* btnFechar = new QPushButton(tr("Fechar"));

    auto* topo = new QHBoxLayout;
    topo->addWidget(m_termo, 1);
    topo->addWidget(m_btnBuscar);
    auto* rodape = new QHBoxLayout;
    rodape->addWidget(m_status, 1);
    rodape->addWidget(btnAdd);
    rodape->addWidget(btnFechar);
    auto* lay = new QVBoxLayout(this);
    lay->addLayout(topo);
    lay->addWidget(m_tabela);
    lay->addLayout(rodape);

    connect(m_btnBuscar, &QPushButton::clicked, this, &GoogleBooksDialog::buscar);
    connect(m_termo, &QLineEdit::returnPressed, this, &GoogleBooksDialog::buscar);
    connect(btnAdd, &QPushButton::clicked, this, &GoogleBooksDialog::adicionarSelecionado);
    connect(m_tabela, &QTableWidget::cellDoubleClicked, this, [this] { adicionarSelecionado(); });
    connect(btnFechar, &QPushButton::clicked, this, &QDialog::accept);
    connect(&m_client, &GoogleBooksClient::resultados, this, &GoogleBooksDialog::mostrar);
    connect(&m_client, &GoogleBooksClient::failed, this, [this](const QString& msg) {
        m_btnBuscar->setEnabled(true);
        m_status->setText(tr("Falha na busca."));
        QMessageBox::warning(this, tr("Sem conexão"), msg);
    });
}

void GoogleBooksDialog::buscar() { BN_TRACE("GoogleBooksDialog::buscar");
    if (m_termo->text().trimmed().isEmpty()) return;
    m_btnBuscar->setEnabled(false);
    m_status->setText(tr("Buscando…"));
    m_client.buscar(m_termo->text());
}

void GoogleBooksDialog::mostrar(const QList<Livro>& livros) { BN_TRACE("GoogleBooksDialog::mostrar");
    m_btnBuscar->setEnabled(true);
    m_resultados = livros;
    m_tabela->setRowCount(livros.size());
    for (int i = 0; i < livros.size(); ++i) {
        const auto& l = livros[i];
        m_tabela->setItem(i, 0, new QTableWidgetItem(l.titulo));
        m_tabela->setItem(i, 1, new QTableWidgetItem(l.autor));
        m_tabela->setItem(i, 2, new QTableWidgetItem(l.genero));
        m_tabela->setItem(i, 3, new QTableWidgetItem(l.anoPublicacao ? QString::number(l.anoPublicacao) : QString()));
        m_tabela->setItem(i, 4, new QTableWidgetItem(l.totalPaginas ? QString::number(l.totalPaginas) : QString()));
    }
    m_status->setText(livros.isEmpty() ? tr("Nenhum resultado.") : tr("%1 resultado(s).").arg(livros.size()));
}

void GoogleBooksDialog::adicionarSelecionado() { BN_TRACE("GoogleBooksDialog::adicionarSelecionado");
    const int row = m_tabela->currentRow();
    if (row < 0 || row >= m_resultados.size()) {
        QMessageBox::information(this, tr("Adicionar"), tr("Selecione um livro da lista."));
        return;
    }
    Livro l = m_resultados[row];
    if (l.genero.isEmpty()) l.genero = QStringLiteral("Outros");
    // abre o formulário já preenchido para conferência (a API pode não informar páginas/autor)
    BookDialog dlg(m_repo.autores(), m_repo.generos(), &l, this);
    dlg.setWindowTitle(tr("Confirmar dados do livro"));
    if (dlg.exec() != QDialog::Accepted) return;
    Livro final = dlg.livro();
    final.id = 0;
    QString erro;
    if (!m_repo.inserir(final, &erro)) {
        QMessageBox::warning(this, tr("Não foi possível adicionar"), erro);
        return;
    }
    m_adicionou = true;
    m_status->setText(tr("\"%1\" adicionado à biblioteca.").arg(final.titulo));
}

}  // namespace booknest
