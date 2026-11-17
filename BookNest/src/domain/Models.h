#pragma once
// Classes de domínio do BookNest (espelham o diagrama de classes do plano de projeto).

#include <QDate>
#include <QList>
#include <QString>

#include "core/ReadingRules.h"

namespace booknest {

using core::Status;

class Autor {
public:
    qint64 id = 0;
    QString nome;
    QString nacionalidade;
    int anoNascimento = 0;  // 0 = não informado
};

class Genero {
public:
    qint64 id = 0;
    QString nome;
    QString descricao;
};

class Livro {
public:
    qint64 id = 0;
    QString titulo;
    QString autor;   // nome do autor (a persistência resolve/cria o registro em "autor")
    QString genero;  // nome do gênero (idem)
    QString isbn;
    int anoPublicacao = 0;  // 0 = não informado
    int totalPaginas = 0;
    QString sinopse;
    QString capaUrl;
    QString origem = QStringLiteral("MANUAL");  // MANUAL | CATALOGO | GOOGLE_BOOKS
};

class Leitura {
public:
    qint64 id = 0;
    qint64 idLivro = 0;
    Status status = Status::QueroLer;
    int paginaAtual = 0;
    QDate dataInicio;
    QDate dataFim;
    int nota = 0;  // 0 = sem nota
    QString resenha;
};

class RegistroProgresso {
public:
    qint64 id = 0;
    qint64 idLeitura = 0;
    QDate data;
    int paginaAtual = 0;
    int paginasLidas = 0;
};

class ListaTematica {
public:
    qint64 id = 0;
    QString nome;
    QString descricao;
    QDate dataCriacao;
    int quantidadeLivros = 0;  // preenchido na listagem
};

}  // namespace booknest
