#include "ReviewDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFormLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>

namespace booknest {

ReviewDialog::ReviewDialog(const QString& titulo, int notaAtual, const QString& resenhaAtual,
                           QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Avaliar livro"));
    setMinimumWidth(420);

    m_nota = new QComboBox;
    for (int i = 1; i <= 5; ++i)
        m_nota->addItem(QString(i, QChar(0x2605)) + QString(5 - i, QChar(0x2606)), i);  // ★☆
    m_nota->setCurrentIndex(notaAtual >= 1 ? notaAtual - 1 : 4);
    m_resenha = new QPlainTextEdit(resenhaAtual);
    m_resenha->setPlaceholderText(tr("Escreva sua resenha ou comentários sobre o livro…"));

    auto* head = new QLabel(QStringLiteral("<b>%1</b>").arg(titulo.toHtmlEscaped()));
    head->setWordWrap(true);
    auto* form = new QFormLayout;
    form->addRow(tr("Nota:"), m_nota);
    form->addRow(tr("Resenha:"), m_resenha);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Save)->setText(tr("Salvar"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("Cancelar"));
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* lay = new QVBoxLayout(this);
    lay->addWidget(head);
    lay->addLayout(form);
    lay->addWidget(buttons);
}

int ReviewDialog::nota() const { return m_nota->currentData().toInt(); }
QString ReviewDialog::resenha() const { return m_resenha->toPlainText(); }

}  // namespace booknest
