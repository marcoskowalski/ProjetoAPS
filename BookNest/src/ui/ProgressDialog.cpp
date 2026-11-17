#include "ProgressDialog.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QFormLayout>
#include <QLabel>
#include <QProgressBar>
#include <QSpinBox>
#include <QVBoxLayout>

#include "core/ReadingRules.h"

namespace booknest {

ProgressDialog::ProgressDialog(const QString& titulo, int paginaAtual, int totalPaginas,
                               QWidget* parent)
    : QDialog(parent), m_total(totalPaginas) {
    setWindowTitle(tr("Atualizar progresso"));
    setMinimumWidth(380);

    m_pagina = new QSpinBox;
    m_pagina->setRange(0, totalPaginas);  // o teto (RN005) também é validado no núcleo
    m_pagina->setValue(paginaAtual);
    m_pagina->setSuffix(tr(" / %1").arg(totalPaginas));
    m_barra = new QProgressBar;
    m_barra->setRange(0, 1000);
    m_info = new QLabel;

    auto* head = new QLabel(QStringLiteral("<b>%1</b>").arg(titulo.toHtmlEscaped()));
    head->setWordWrap(true);
    auto* form = new QFormLayout;
    form->addRow(tr("Página atual:"), m_pagina);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Atualizar Progresso"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("Cancelar"));
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* lay = new QVBoxLayout(this);
    lay->addWidget(head);
    lay->addLayout(form);
    lay->addWidget(m_barra);
    lay->addWidget(m_info);
    lay->addWidget(buttons);

    connect(m_pagina, qOverload<int>(&QSpinBox::valueChanged), this, [this] { atualizar(); });
    atualizar();
}

int ProgressDialog::novaPagina() const { return m_pagina->value(); }

void ProgressDialog::atualizar() {
    const double pct = core::progressPercent(m_pagina->value(), m_total);
    m_barra->setValue(static_cast<int>(pct * 10));
    m_barra->setFormat(QStringLiteral("%1%").arg(pct, 0, 'f', 1));
    m_info->setText(tr("%1 de %2 páginas — %3% concluído")
                        .arg(m_pagina->value()).arg(m_total).arg(pct, 0, 'f', 1));
}

}  // namespace booknest
