#include "LeituraRepository.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "DatabaseManager.h"

namespace booknest {

namespace {
QSqlDatabase db() { return DatabaseManager::instance().database(); }
const QString kIso = QStringLiteral("yyyy-MM-dd");

int totalPaginas(qint64 idLivro) {
    QSqlQuery q(db());
    q.prepare(QStringLiteral("SELECT total_paginas FROM livro WHERE id = ?"));
    q.addBindValue(idLivro);
    return (q.exec() && q.next()) ? q.value(0).toInt() : 0;
}
}  // namespace

std::optional<Leitura> LeituraRepository::porLivro(qint64 idLivro) const {
    QSqlQuery q(db());
    q.prepare(QStringLiteral(
        "SELECT id, id_livro, status, pagina_atual, data_inicio, data_fim, nota, resenha "
        "FROM leitura WHERE id_livro = ?"));
    q.addBindValue(idLivro);
    if (!q.exec() || !q.next()) return std::nullopt;
    Leitura l;
    l.id = q.value(0).toLongLong();
    l.idLivro = q.value(1).toLongLong();
    l.status = core::statusFromCode(q.value(2).toString().toStdString()).value_or(Status::QueroLer);
    l.paginaAtual = q.value(3).toInt();
    l.dataInicio = QDate::fromString(q.value(4).toString(), kIso);
    l.dataFim = QDate::fromString(q.value(5).toString(), kIso);
    l.nota = q.value(6).isNull() ? 0 : q.value(6).toInt();
    l.resenha = q.value(7).toString();
    return l;
}

bool LeituraRepository::alterarStatus(qint64 idLivro, Status novo, QString* erro) {
    auto l = porLivro(idLivro);
    if (!l) {
        if (erro) *erro = QStringLiteral("Livro não encontrado.");
        return false;
    }
    const int total = totalPaginas(idLivro);
    const auto ch = core::applyStatusChange(l->status, novo, l->paginaAtual, total,
                                            l->dataInicio.isValid());
    if (!ch.result.ok) {
        if (erro) *erro = QString::fromStdString(ch.result.message);
        return false;
    }
    const QString hoje = QDate::currentDate().toString(kIso);
    QString sql = QStringLiteral("UPDATE leitura SET status = ?, pagina_atual = ?");
    if (ch.setStartDate) sql += QStringLiteral(", data_inicio = ?");
    if (ch.setEndDate) sql += QStringLiteral(", data_fim = ?");
    if (ch.clearEndDate) sql += QStringLiteral(", data_fim = NULL");
    sql += QStringLiteral(" WHERE id_livro = ?");

    QSqlQuery q(db());
    q.prepare(sql);
    q.addBindValue(QString::fromStdString(core::statusCode(novo)));
    q.addBindValue(ch.newPage);
    if (ch.setStartDate) q.addBindValue(hoje);
    if (ch.setEndDate) q.addBindValue(hoje);
    q.addBindValue(idLivro);
    if (!q.exec()) {
        if (erro) *erro = q.lastError().text();
        return false;
    }
    return true;
}

bool LeituraRepository::registrarProgresso(qint64 idLivro, int novaPagina,
                                           core::ProgressOutcome* resultado, QString* erro) {
    auto l = porLivro(idLivro);
    if (!l) {
        if (erro) *erro = QStringLiteral("Livro não encontrado.");
        return false;
    }
    const int total = totalPaginas(idLivro);
    const auto out = core::applyProgress(l->status, l->paginaAtual, novaPagina, total);
    if (resultado) *resultado = out;
    if (!out.result.ok) {
        if (erro) *erro = QString::fromStdString(out.result.message);
        return false;
    }

    QSqlDatabase d = db();
    if (!d.transaction()) {
        if (erro) *erro = d.lastError().text();
        return false;
    }
    const QString hoje = QDate::currentDate().toString(kIso);

    QSqlQuery u(d);
    if (out.finished) {
        u.prepare(QStringLiteral(
            "UPDATE leitura SET pagina_atual = ?, status = 'LIDO', data_fim = ? WHERE id_livro = ?"));
        u.addBindValue(novaPagina);
        u.addBindValue(hoje);
    } else {
        u.prepare(QStringLiteral("UPDATE leitura SET pagina_atual = ? WHERE id_livro = ?"));
        u.addBindValue(novaPagina);
    }
    u.addBindValue(idLivro);
    if (!u.exec()) {
        if (erro) *erro = u.lastError().text();
        d.rollback();
        return false;
    }

    QSqlQuery r(d);
    r.prepare(QStringLiteral("INSERT INTO registro_progresso(id_leitura, data_registro, "
                             "pagina_atual, paginas_lidas) VALUES (?, ?, ?, ?)"));
    r.addBindValue(l->id);
    r.addBindValue(hoje);
    r.addBindValue(novaPagina);
    r.addBindValue(out.pagesRead);
    if (!r.exec()) {
        if (erro) *erro = r.lastError().text();
        d.rollback();
        return false;
    }
    if (!d.commit()) {
        if (erro) *erro = d.lastError().text();
        return false;
    }
    return true;
}

bool LeituraRepository::avaliar(qint64 idLivro, int nota, const QString& resenha, QString* erro) {
    auto l = porLivro(idLivro);
    if (!l) {
        if (erro) *erro = QStringLiteral("Livro não encontrado.");
        return false;
    }
    const auto v = core::validateRating(l->status, nota);
    if (!v.ok) {
        if (erro) *erro = QString::fromStdString(v.message);
        return false;
    }
    QSqlQuery q(db());
    q.prepare(QStringLiteral("UPDATE leitura SET nota = ?, resenha = ? WHERE id_livro = ?"));
    q.addBindValue(nota);
    q.addBindValue(resenha.trimmed());
    q.addBindValue(idLivro);
    if (!q.exec()) {
        if (erro) *erro = q.lastError().text();
        return false;
    }
    return true;
}

QList<RegistroProgresso> LeituraRepository::historico(qint64 idLivro) const {
    QList<RegistroProgresso> out;
    QSqlQuery q(db());
    q.prepare(QStringLiteral(
        "SELECT p.id, p.id_leitura, p.data_registro, p.pagina_atual, p.paginas_lidas "
        "FROM registro_progresso p JOIN leitura l ON l.id = p.id_leitura "
        "WHERE l.id_livro = ? ORDER BY p.data_registro DESC, p.id DESC"));
    q.addBindValue(idLivro);
    if (!q.exec()) return out;
    while (q.next()) {
        RegistroProgresso r;
        r.id = q.value(0).toLongLong();
        r.idLeitura = q.value(1).toLongLong();
        r.data = QDate::fromString(q.value(2).toString(), kIso);
        r.paginaAtual = q.value(3).toInt();
        r.paginasLidas = q.value(4).toInt();
        out << r;
    }
    return out;
}

}  // namespace booknest
