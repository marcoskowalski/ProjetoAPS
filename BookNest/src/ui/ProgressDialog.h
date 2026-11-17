#pragma once
#include <QDialog>

class QSpinBox;
class QLabel;
class QProgressBar;

namespace booknest {

// Registro de progresso de leitura (UC004).
class ProgressDialog : public QDialog {
    Q_OBJECT
public:
    ProgressDialog(const QString& titulo, int paginaAtual, int totalPaginas, QWidget* parent = nullptr);
    int novaPagina() const;

private:
    void atualizar();
    int m_total;
    QSpinBox* m_pagina;
    QProgressBar* m_barra;
    QLabel* m_info;
};

}  // namespace booknest
