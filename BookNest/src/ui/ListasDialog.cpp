#include "ListasDialog.h"
#include "core/Trace.h"

#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace booknest {

ListasDialog::ListasDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Listas temáticas"));
    resize(720, 440);

    m_wListas = new QListWidget;
    m_wLivros = new QListWidget;
    m_desc = new QLabel;
    m_desc->setWordWrap(true);

    auto* bNova = new QPushButton(tr("Nova lista…"));
    auto* bRemL = new QPushButton(tr("Excluir lista"));
    auto* bAdd = new QPushButton(tr("Adicionar livro…"));
    auto* bRemB = new QPushButton(tr("Remover da lista"));

    auto* esq = new QVBoxLayout;
    esq->addWidget(new QLabel(tr("<b>Minhas listas</b>")));
    esq->addWidget(m_wListas);
    auto* esqBtn = new QHBoxLayout;
    esqBtn->addWidget(bNova);
    esqBtn->addWidget(bRemL);
    esq->addLayout(esqBtn);

    auto* dir = new QVBoxLayout;
    dir->addWidget(new QLabel(tr("<b>Livros da lista</b>")));
    dir->addWidget(m_desc);
    dir->addWidget(m_wLivros);
    auto* dirBtn = new QHBoxLayout;
    dirBtn->addWidget(bAdd);
    dirBtn->addWidget(bRemB);
    dir->addLayout(dirBtn);

    auto* lay = new QHBoxLayout(this);
    lay->addLayout(esq, 1);
    lay->addLayout(dir, 2);

    connect(bNova, &QPushButton::clicked, this, &ListasDialog::novaLista);
    connect(bRemL, &QPushButton::clicked, this, &ListasDialog::removerLista);
    connect(bAdd, &QPushButton::clicked, this, &ListasDialog::adicionarLivro);
    connect(bRemB, &QPushButton::clicked, this, &ListasDialog::removerLivro);
    connect(m_wListas, &QListWidget::currentRowChanged, this, [this] { recarregarLivros(); });

    recarregarListas();
}

qint64 ListasDialog::listaAtual() const {
    auto* it = m_wListas->currentItem();
    return it ? it->data(Qt::UserRole).toLongLong() : 0;
}

void ListasDialog::recarregarListas(qint64 selecionar) { BN_TRACE("ListasDialog::recarregarListas");
    m_wListas->blockSignals(true);
    m_wListas->clear();
    int row = 0, i = 0;
    for (const auto& l : m_listas.listar()) {
        auto* it = new QListWidgetItem(QStringLiteral("%1 (%2)").arg(l.nome).arg(l.quantidadeLivros));
        it->setData(Qt::UserRole, l.id);
        it->setData(Qt::UserRole + 1, l.descricao);
        m_wListas->addItem(it);
        if (l.id == selecionar) row = i;
        ++i;
    }
    m_wListas->blockSignals(false);
    if (m_wListas->count() > 0) m_wListas->setCurrentRow(row);
    recarregarLivros();
}

void ListasDialog::recarregarLivros() {
    m_wLivros->clear();
    auto* it = m_wListas->currentItem();
    m_desc->setText(it ? it->data(Qt::UserRole + 1).toString() : QString());
    if (!it) return;
    for (const auto& l : m_listas.livros(listaAtual())) {
        auto* li = new QListWidgetItem(QStringLiteral("%1 — %2").arg(l.titulo, l.autor));
        li->setData(Qt::UserRole, l.id);
        m_wLivros->addItem(li);
    }
}

void ListasDialog::novaLista() { BN_TRACE("ListasDialog::novaLista");
    bool ok = false;
    const QString nome = QInputDialog::getText(this, tr("Nova lista"), tr("Nome da lista:"),
                                               QLineEdit::Normal, QString(), &ok);
    if (!ok) return;
    const QString desc = QInputDialog::getText(this, tr("Nova lista"), tr("Descrição (opcional):"),
                                               QLineEdit::Normal, QString(), &ok);
    if (!ok) return;
    QString erro;
    auto id = m_listas.criar(nome, desc, &erro);
    if (!id) {
        QMessageBox::warning(this, tr("Não foi possível criar"), erro);
        return;
    }
    recarregarListas(*id);
}

void ListasDialog::removerLista() {
    const qint64 id = listaAtual();
    if (id == 0) return;
    if (QMessageBox::question(this, tr("Excluir lista"),
                              tr("Excluir a lista selecionada? Os livros continuam na biblioteca.")) !=
        QMessageBox::Yes)
        return;
    QString erro;
    if (!m_listas.remover(id, &erro)) QMessageBox::warning(this, tr("Erro"), erro);
    recarregarListas();
}

void ListasDialog::adicionarLivro() { BN_TRACE("ListasDialog::adicionarLivro");
    const qint64 id = listaAtual();
    if (id == 0) {
        QMessageBox::information(this, tr("Listas"), tr("Crie ou selecione uma lista primeiro."));
        return;
    }
    const auto livros = m_livros.todos();
    QStringList rotulos;
    for (const auto& l : livros) rotulos << QStringLiteral("%1 — %2").arg(l.titulo, l.autor);
    bool ok = false;
    const QString escolhido =
        QInputDialog::getItem(this, tr("Adicionar livro"), tr("Livro:"), rotulos, 0, false, &ok);
    if (!ok) return;
    const int idx = rotulos.indexOf(escolhido);
    if (idx < 0) return;
    QString erro;
    if (!m_listas.adicionarLivro(id, livros[idx].id, &erro))
        QMessageBox::warning(this, tr("Não foi possível adicionar"), erro);
    recarregarListas(id);
}

void ListasDialog::removerLivro() {
    auto* it = m_wLivros->currentItem();
    const qint64 id = listaAtual();
    if (!it || id == 0) return;
    QString erro;
    if (!m_listas.removerLivro(id, it->data(Qt::UserRole).toLongLong(), &erro))
        QMessageBox::warning(this, tr("Erro"), erro);
    recarregarListas(id);
}

}  // namespace booknest
