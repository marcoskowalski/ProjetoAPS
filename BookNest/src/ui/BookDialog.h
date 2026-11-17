#pragma once
#include <QDialog>

#include "domain/Models.h"

class QLineEdit;
class QComboBox;
class QSpinBox;
class QPlainTextEdit;

namespace booknest {

// Formulário de cadastro/edição de livro (UC001 e UC002).
class BookDialog : public QDialog {
    Q_OBJECT
public:
    explicit BookDialog(const QStringList& autores, const QStringList& generos,
                        const Livro* existente = nullptr, QWidget* parent = nullptr);
    Livro livro() const;

public slots:
    void accept() override;  // valida (RN001, RN002) e emite alerta descritivo em caso de erro (FE01)

private:
    qint64 m_id = 0;
    QString m_origem = QStringLiteral("MANUAL");
    QString m_capa;
    QLineEdit* m_titulo;
    QLineEdit* m_autor;
    QComboBox* m_genero;
    QSpinBox* m_paginas;
    QLineEdit* m_isbn;
    QSpinBox* m_ano;
    QPlainTextEdit* m_sinopse;
};

}  // namespace booknest
