#pragma once
#include <optional>

#include "domain/Models.h"

namespace booknest {

// Listas temáticas personalizadas (RF007).
class ListaRepository {
public:
    std::optional<qint64> criar(const QString& nome, const QString& descricao, QString* erro);
    bool remover(qint64 idLista, QString* erro);
    QList<ListaTematica> listar() const;

    bool adicionarLivro(qint64 idLista, qint64 idLivro, QString* erro);
    bool removerLivro(qint64 idLista, qint64 idLivro, QString* erro);
    QList<Livro> livros(qint64 idLista) const;  // id, titulo, autor
};

}  // namespace booknest
