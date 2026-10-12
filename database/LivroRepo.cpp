#include "LivroRepo.h"
#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

bool LivroRepo::inserir(const Livro &livro) {
    QSqlQuery query;
    query.prepare("INSERT INTO livros (titulo, autor, genero, paginas_totais, ano_publicacao, status) "
                  "VALUES (:titulo, :autor, :genero, :paginas, :ano, :status)");

    query.bindValue(":titulo", livro.getTitulo());
    query.bindValue(":autor", livro.getAutor());
    query.bindValue(":genero", livro.getGenero());
    query.bindValue(":paginas", livro.getPaginasTotais());
    query.bindValue(":ano", livro.getAnoPublicacao());
    query.bindValue(":status", livro.getStatus());

    if (!query.exec()) {
        qDebug() << "Erro ao inserir livro no repositório:" << query.lastError().text();
        return false;
    }
    return true;
}

QList<Livro> LivroRepo::listarTodos() {
    QList<Livro> lista;
    QSqlQuery query("SELECT id, titulo, autor, genero, paginas_totais, ano_publicacao, status FROM livros");

    while (query.next()) {
        int id = query.value(0).toInt();
        QString titulo = query.value(1).toString();
        QString autor = query.value(2).toString();
        QString genero = query.value(3).toString();
        int paginas = query.value(4).toInt();
        int ano = query.value(5).toInt();
        QString status = query.value(6).toString();

        // Cria o objeto Livro usando o construtor parametrizado que você definiu
        Livro livro(id, titulo, autor, genero, paginas, ano, status);
        lista.append(livro);
    }

    return lista;
}
