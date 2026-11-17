#pragma once
#include <QDialog>

#include "db/LivroRepository.h"
#include "domain/Models.h"
#include "net/GoogleBooksClient.h"

class QLineEdit;
class QTableWidget;
class QLabel;
class QPushButton;

namespace booknest {

// Pesquisa livros na internet (Google Books) e adiciona à biblioteca após conferência.
class GoogleBooksDialog : public QDialog {
    Q_OBJECT
public:
    explicit GoogleBooksDialog(QWidget* parent = nullptr);
    bool adicionouLivro() const { return m_adicionou; }

private:
    void buscar();
    void mostrar(const QList<Livro>& livros);
    void adicionarSelecionado();

    GoogleBooksClient m_client;
    LivroRepository m_repo;
    QList<Livro> m_resultados;
    bool m_adicionou = false;
    QLineEdit* m_termo;
    QPushButton* m_btnBuscar;
    QTableWidget* m_tabela;
    QLabel* m_status;
};

}  // namespace booknest
