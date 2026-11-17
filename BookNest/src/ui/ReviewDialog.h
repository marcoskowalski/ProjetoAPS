#pragma once
#include <QDialog>

class QComboBox;
class QPlainTextEdit;

namespace booknest {

// Nota (1 a 5 estrelas) e resenha de um livro lido (RF005).
class ReviewDialog : public QDialog {
    Q_OBJECT
public:
    ReviewDialog(const QString& titulo, int notaAtual, const QString& resenhaAtual,
                 QWidget* parent = nullptr);
    int nota() const;
    QString resenha() const;

private:
    QComboBox* m_nota;
    QPlainTextEdit* m_resenha;
};

}  // namespace booknest
