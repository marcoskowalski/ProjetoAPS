#pragma once
#include <QDialog>

#include "db/LivroRepository.h"
#include "db/ListaRepository.h"

class QListWidget;
class QLabel;

namespace booknest {

// Gerenciamento de listas temáticas personalizadas (RF007).
class ListasDialog : public QDialog {
    Q_OBJECT
public:
    explicit ListasDialog(QWidget* parent = nullptr);

private:
    void recarregarListas(qint64 selecionar = 0);
    void recarregarLivros();
    qint64 listaAtual() const;
    void novaLista();
    void removerLista();
    void adicionarLivro();
    void removerLivro();

    ListaRepository m_listas;
    LivroRepository m_livros;
    QListWidget* m_wListas;
    QListWidget* m_wLivros;
    QLabel* m_desc;
};

}  // namespace booknest
