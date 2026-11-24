#include "LivroRepository.h"
#include "core/Trace.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "DatabaseManager.h"

namespace booknest {

namespace {
QSqlDatabase db() { return DatabaseManager::instance().database(); }

QVariant nullIfEmpty(const QString& s) {
    return s.trimmed().isEmpty() ? QVariant(QMetaType(QMetaType::QString)) : QVariant(s.trimmed());
}
QVariant nullIfZero(int v) { return v == 0 ? QVariant(QMetaType(QMetaType::Int)) : QVariant(v); }

// Retorna o id do registro com esse nome, criando-o se necessário.
std::optional<qint64> idPorNome(const QString& tabela, const QString& nome, QString* erro) {
    QSqlQuery sel(db());
    sel.prepare(QStringLiteral("SELECT id FROM %1 WHERE nome = ? COLLATE NOCASE").arg(tabela));
    sel.addBindValue(nome.trimmed());
    if (sel.exec() && sel.next()) return sel.value(0).toLongLong();

    QSqlQuery ins(db());
    ins.prepare(QStringLiteral("INSERT INTO %1(nome) VALUES (?)").arg(tabela));
    ins.addBindValue(nome.trimmed());
    if (!ins.exec()) {
        if (erro) *erro = ins.lastError().text();
        return std::nullopt;
    }
    return ins.lastInsertId().toLongLong();
}

QString traduzErro(const QSqlError& e) {
    if (e.text().contains(QStringLiteral("UNIQUE"), Qt::CaseInsensitive))
        return QStringLiteral("Já existe um livro com esse título e autor na biblioteca.");
    return e.text();
}
}  // namespace

std::optional<qint64> LivroRepository::inserir(const Livro& l, QString* erro) { BN_TRACE("LivroRepository::inserir");
    QSqlDatabase d = db();
    if (!d.transaction()) {
        if (erro) *erro = d.lastError().text();
        return std::nullopt;
    }
    auto idAutor = idPorNome(QStringLiteral("autor"), l.autor, erro);
    auto idGenero = idAutor ? idPorNome(QStringLiteral("genero"), l.genero, erro) : std::nullopt;
    if (!idAutor || !idGenero) {
        d.rollback();
        return std::nullopt;
    }

    QSqlQuery q(d);
    q.prepare(QStringLiteral(
        "INSERT INTO livro(titulo, isbn, ano_publicacao, total_paginas, sinopse, capa_url, origem, "
        "id_autor, id_genero) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    q.addBindValue(l.titulo.trimmed());
    q.addBindValue(nullIfEmpty(l.isbn));
    q.addBindValue(nullIfZero(l.anoPublicacao));
    q.addBindValue(l.totalPaginas);
    q.addBindValue(nullIfEmpty(l.sinopse));
    q.addBindValue(nullIfEmpty(l.capaUrl));
    q.addBindValue(l.origem);
    q.addBindValue(*idAutor);
    q.addBindValue(*idGenero);
    if (!q.exec()) {
        if (erro) *erro = traduzErro(q.lastError());
        d.rollback();
        return std::nullopt;
    }
    const qint64 idLivro = q.lastInsertId().toLongLong();

    QSqlQuery r(d);
    r.prepare(QStringLiteral("INSERT INTO leitura(id_livro) VALUES (?)"));
    r.addBindValue(idLivro);
    if (!r.exec()) {
        if (erro) *erro = r.lastError().text();
        d.rollback();
        return std::nullopt;
    }
    if (!d.commit()) {
        if (erro) *erro = d.lastError().text();
        return std::nullopt;
    }
    return idLivro;
}

bool LivroRepository::atualizar(const Livro& l, QString* erro) { BN_TRACE("LivroRepository::atualizar");
    QSqlDatabase d = db();
    if (!d.transaction()) {
        if (erro) *erro = d.lastError().text();
        return false;
    }
    auto idAutor = idPorNome(QStringLiteral("autor"), l.autor, erro);
    auto idGenero = idAutor ? idPorNome(QStringLiteral("genero"), l.genero, erro) : std::nullopt;
    if (!idAutor || !idGenero) {
        d.rollback();
        return false;
    }
    QSqlQuery q(d);
    q.prepare(QStringLiteral(
        "UPDATE livro SET titulo = ?, isbn = ?, ano_publicacao = ?, total_paginas = ?, sinopse = ?, "
        "id_autor = ?, id_genero = ? WHERE id = ?"));
    q.addBindValue(l.titulo.trimmed());
    q.addBindValue(nullIfEmpty(l.isbn));
    q.addBindValue(nullIfZero(l.anoPublicacao));
    q.addBindValue(l.totalPaginas);
    q.addBindValue(nullIfEmpty(l.sinopse));
    q.addBindValue(*idAutor);
    q.addBindValue(*idGenero);
    q.addBindValue(l.id);
    if (!q.exec()) {
        if (erro) *erro = traduzErro(q.lastError());
        d.rollback();
        return false;
    }
    if (q.numRowsAffected() == 0) {
        if (erro) *erro = QStringLiteral("O livro não existe mais na biblioteca.");
        d.rollback();
        return false;
    }
    // se o novo total de páginas ficou abaixo da página atual, limita a página atual
    QSqlQuery adj(d);
    adj.prepare(QStringLiteral(
        "UPDATE leitura SET pagina_atual = ? WHERE id_livro = ? AND pagina_atual > ?"));
    adj.addBindValue(l.totalPaginas);
    adj.addBindValue(l.id);
    adj.addBindValue(l.totalPaginas);
    adj.exec();
    if (!d.commit()) {
        if (erro) *erro = d.lastError().text();
        return false;
    }
    return true;
}

bool LivroRepository::remover(qint64 idLivro, QString* erro) { BN_TRACE("LivroRepository::remover");
    QSqlQuery q(db());
    q.prepare(QStringLiteral("DELETE FROM livro WHERE id = ?"));  // ON DELETE CASCADE limpa o resto
    q.addBindValue(idLivro);
    if (!q.exec()) {
        if (erro) *erro = q.lastError().text();
        return false;
    }
    if (q.numRowsAffected() == 0) {
        if (erro) *erro = QStringLiteral("O registro não existe ou já foi removido.");  // FE01 UC003
        return false;
    }
    return true;
}

std::optional<Livro> LivroRepository::buscar(qint64 idLivro) const { BN_TRACE("LivroRepository::buscar");
    QSqlQuery q(db());
    q.prepare(QStringLiteral(
        "SELECT l.id, l.titulo, a.nome, g.nome, l.isbn, l.ano_publicacao, l.total_paginas, "
        "l.sinopse, l.capa_url, l.origem FROM livro l JOIN autor a ON a.id = l.id_autor "
        "JOIN genero g ON g.id = l.id_genero WHERE l.id = ?"));
    q.addBindValue(idLivro);
    if (!q.exec() || !q.next()) return std::nullopt;
    Livro l;
    l.id = q.value(0).toLongLong();
    l.titulo = q.value(1).toString();
    l.autor = q.value(2).toString();
    l.genero = q.value(3).toString();
    l.isbn = q.value(4).toString();
    l.anoPublicacao = q.value(5).toInt();
    l.totalPaginas = q.value(6).toInt();
    l.sinopse = q.value(7).toString();
    l.capaUrl = q.value(8).toString();
    l.origem = q.value(9).toString();
    return l;
}

QStringList LivroRepository::autores() const { BN_TRACE("LivroRepository::autores");
    QStringList out;
    QSqlQuery q(QStringLiteral("SELECT nome FROM autor ORDER BY nome COLLATE NOCASE"), db());
    while (q.next()) out << q.value(0).toString();
    return out;
}

QStringList LivroRepository::generos() const { BN_TRACE("LivroRepository::generos");
    QStringList out;
    QSqlQuery q(QStringLiteral("SELECT nome FROM genero ORDER BY nome COLLATE NOCASE"), db());
    while (q.next()) out << q.value(0).toString();
    return out;
}

QList<Livro> LivroRepository::todos() const { BN_TRACE("LivroRepository::todos");
    QList<Livro> out;
    QSqlQuery q(QStringLiteral("SELECT l.id, l.titulo, a.nome FROM livro l "
                               "JOIN autor a ON a.id = l.id_autor ORDER BY l.titulo COLLATE NOCASE"),
                db());
    while (q.next()) {
        Livro l;
        l.id = q.value(0).toLongLong();
        l.titulo = q.value(1).toString();
        l.autor = q.value(2).toString();
        out << l;
    }
    return out;
}

}  // namespace booknest
