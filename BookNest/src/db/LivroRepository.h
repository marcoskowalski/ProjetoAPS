#pragma once
#include <optional>

#include <QStringList>

#include "domain/Models.h"

namespace booknest {

// CRUD de livros (UC001, UC002, UC003). Autor e gênero são criados sob demanda.
class LivroRepository {
public:
    std::optional<qint64> inserir(const Livro& livro, QString* erro);   // cria também a "leitura"
    bool atualizar(const Livro& livro, QString* erro);
    bool remover(qint64 idLivro, QString* erro);
    std::optional<Livro> buscar(qint64 idLivro) const;

    QStringList autores() const;
    QStringList generos() const;
    QList<Livro> todos() const;  // id, titulo e autor (para seletores)
};

}  // namespace booknest
