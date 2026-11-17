#include "BookDialog.h"

#include <QComboBox>
#include <QCompleter>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QVBoxLayout>

#include "core/ReadingRules.h"

namespace booknest {

BookDialog::BookDialog(const QStringList& autores, const QStringList& generos,
                       const Livro* existente, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(existente ? tr("Editar livro") : tr("Cadastrar livro"));
    setMinimumWidth(440);

    m_titulo = new QLineEdit;
    m_autor = new QLineEdit;
    m_autor->setCompleter(new QCompleter(autores, this));
    m_genero = new QComboBox;
    m_genero->setEditable(true);
    m_genero->addItems(generos);
    m_genero->setCurrentIndex(-1);
    m_paginas = new QSpinBox;
    m_paginas->setRange(0, 50000);
    m_paginas->setSpecialValueText(tr("(informe)"));
    m_isbn = new QLineEdit;
    m_isbn->setPlaceholderText(tr("Opcional — ISBN-10 ou ISBN-13"));
    m_ano = new QSpinBox;
    m_ano->setRange(0, 2100);
    m_ano->setSpecialValueText(tr("(não informado)"));
    m_sinopse = new QPlainTextEdit;
    m_sinopse->setFixedHeight(90);

    auto* form = new QFormLayout;
    form->addRow(tr("Título *"), m_titulo);
    form->addRow(tr("Autor *"), m_autor);
    form->addRow(tr("Gênero *"), m_genero);
    form->addRow(tr("Total de páginas *"), m_paginas);
    form->addRow(tr("ISBN"), m_isbn);
    form->addRow(tr("Ano de publicação"), m_ano);
    form->addRow(tr("Sinopse"), m_sinopse);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Save)->setText(tr("Salvar"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("Cancelar"));  // FA01
    connect(buttons, &QDialogButtonBox::accepted, this, &BookDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* lay = new QVBoxLayout(this);
    lay->addLayout(form);
    lay->addWidget(buttons);

    if (existente) {
        m_id = existente->id;
        m_origem = existente->origem;
        m_capa = existente->capaUrl;
        m_titulo->setText(existente->titulo);
        m_autor->setText(existente->autor);
        m_genero->setCurrentText(existente->genero);
        m_paginas->setValue(existente->totalPaginas);
        m_isbn->setText(existente->isbn);
        m_ano->setValue(existente->anoPublicacao);
        m_sinopse->setPlainText(existente->sinopse);
    }
}

Livro BookDialog::livro() const {
    Livro l;
    l.id = m_id;
    l.titulo = m_titulo->text().trimmed();
    l.autor = m_autor->text().trimmed();
    l.genero = m_genero->currentText().trimmed();
    l.totalPaginas = m_paginas->value();
    l.isbn = m_isbn->text().trimmed();
    l.anoPublicacao = m_ano->value();
    l.sinopse = m_sinopse->toPlainText().trimmed();
    l.capaUrl = m_capa;
    l.origem = m_origem;
    return l;
}

void BookDialog::accept() {
    const Livro l = livro();
    const auto r = core::validateBook(l.titulo.toStdString(), l.autor.toStdString(),
                                      l.genero.toStdString(), l.totalPaginas, l.anoPublicacao,
                                      l.isbn.toStdString());
    if (!r.ok) {
        QMessageBox::warning(this, tr("Dados inválidos"), QString::fromStdString(r.message));
        return;  // permanece no formulário
    }
    QDialog::accept();
}

}  // namespace booknest
