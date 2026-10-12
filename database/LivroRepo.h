#ifndef LIVROREPO_H
#define LIVROREPO_H

#include <QList>
#include "../models/Livro.h"

class LivroRepo {
public:
    static bool inserir(const Livro &livro);
    static QList<Livro> listarTodos();
};

#endif // LIVROREPO_H