#include "ListaRepository.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "DatabaseManager.h"

namespace booknest {

namespace {
QSqlDatabase db() { return DatabaseManager::instance().database(); }
const QString kIso = QStringLiteral("yyyy-MM-dd");
QString traduz(const QSqlError& e, const QString& msgUnique) {
    return e.text().contains(QStringLiteral("UNIQUE"), Qt::CaseInsensitive) ? msgUnique : e.text();
}
}  // namespace

std::optional<qint64> ListaRepository::criar(const QString& nome, const QString& descricao,
                                             QString* erro) {
    if (nome.trimmed().isEmpty()) {
        if (erro) *erro = QStringLiteral("O nome da lista é obrigatório.");
        return std::nullopt;
    }
    QSqlQuery q(db());
    q.prepare(QStringLiteral(
        "INSERT INTO lista_tematica(nome, descricao, data_criacao) VALUES (?, ?, ?)"));
    q.addBindValue(nome.trimmed());
    q.addBindValue(descricao.trimmed());
    q.addBindValue(QDate::currentDate().toString(kIso));
    if (!q.exec()) {
        if (erro) *erro = traduz(q.lastError(), QStringLiteral("Já existe uma lista com esse nome."));
        return std::nullopt;
    }
    return q.lastInsertId().toLongLong();
}

bool ListaRepository::remover(qint64 idLista, QString* erro) {
    QSqlQuery q(db());
    q.prepare(QStringLiteral("DELETE FROM lista_tematica WHERE id = ?"));
    q.addBindValue(idLista);
    if (!q.exec()) {
        if (erro) *erro = q.lastError().text();
        return false;
    }
    return true;
}

QList<ListaTematica> ListaRepository::listar() const {
    QList<ListaTematica> out;
    QSqlQuery q(QStringLiteral(
                    "SELECT t.id, t.nome, t.descricao, t.data_criacao, COUNT(ll.id_livro) "
                    "FROM lista_tematica t LEFT JOIN lista_livro ll ON ll.id_lista = t.id "
                    "GROUP BY t.id ORDER BY t.nome COLLATE NOCASE"),
                db());
    while (q.next()) {
        ListaTematica t;
        t.id = q.value(0).toLongLong();
        t.nome = q.value(1).toString();
        t.descricao = q.value(2).toString();
        t.dataCriacao = QDate::fromString(q.value(3).toString(), kIso);
        t.quantidadeLivros = q.value(4).toInt();
        out << t;
    }
    return out;
}

bool ListaRepository::adicionarLivro(qint64 idLista, qint64 idLivro, QString* erro) {
    QSqlQuery q(db());
    q.prepare(QStringLiteral(
        "INSERT INTO lista_livro(id_lista, id_livro, data_adicao) VALUES (?, ?, ?)"));
    q.addBindValue(idLista);
    q.addBindValue(idLivro);
    q.addBindValue(QDate::currentDate().toString(kIso));
    if (!q.exec()) {
        if (erro) *erro = traduz(q.lastError(), QStringLiteral("Esse livro já está na lista."));
        return false;
    }
    return true;
}

bool ListaRepository::removerLivro(qint64 idLista, qint64 idLivro, QString* erro) {
    QSqlQuery q(db());
    q.prepare(QStringLiteral("DELETE FROM lista_livro WHERE id_lista = ? AND id_livro = ?"));
    q.addBindValue(idLista);
    q.addBindValue(idLivro);
    if (!q.exec()) {
        if (erro) *erro = q.lastError().text();
        return false;
    }
    return true;
}

QList<Livro> ListaRepository::livros(qint64 idLista) const {
    QList<Livro> out;
    QSqlQuery q(db());
    q.prepare(QStringLiteral(
        "SELECT l.id, l.titulo, a.nome FROM lista_livro ll JOIN livro l ON l.id = ll.id_livro "
        "JOIN autor a ON a.id = l.id_autor WHERE ll.id_lista = ? ORDER BY l.titulo COLLATE NOCASE"));
    q.addBindValue(idLista);
    if (!q.exec()) return out;
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
