#include "EstatisticaRepository.h"
#include "core/Trace.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include "DatabaseManager.h"

namespace booknest {

namespace {
QSqlDatabase db() { return DatabaseManager::instance().database(); }
}  // namespace

QList<int> EstatisticaRepository::anosComLeitura() const { BN_TRACE("EstatisticaRepository::anosComLeitura");
    QList<int> out;
    QSqlQuery q(QStringLiteral(
                    "SELECT DISTINCT CAST(substr(data_fim, 1, 4) AS INTEGER) AS ano FROM leitura "
                    "WHERE status = 'LIDO' AND data_fim IS NOT NULL ORDER BY ano DESC"),
                db());
    while (q.next()) out << q.value(0).toInt();
    return out;
}

EstatisticaAnual EstatisticaRepository::anual(int ano) const { BN_TRACE("EstatisticaRepository::anual");
    EstatisticaAnual e;
    e.ano = ano;
    const QString anoStr = QString::number(ano);

    QSqlQuery q(db());
    q.prepare(QStringLiteral(
        "SELECT COUNT(*), COALESCE(SUM(l.total_paginas), 0), AVG(r.nota) "
        "FROM leitura r JOIN livro l ON l.id = r.id_livro "
        "WHERE r.status = 'LIDO' AND substr(r.data_fim, 1, 4) = ?"));
    q.addBindValue(anoStr);
    if (q.exec() && q.next()) {
        e.livrosLidos = q.value(0).toInt();
        e.paginasLidas = q.value(1).toInt();
        e.notaMedia = q.value(2).isNull() ? 0.0 : q.value(2).toDouble();
    }

    QSqlQuery g(db());
    g.prepare(QStringLiteral(
        "SELECT ge.nome, COUNT(*) AS n FROM leitura r JOIN livro l ON l.id = r.id_livro "
        "JOIN genero ge ON ge.id = l.id_genero "
        "WHERE r.status = 'LIDO' AND substr(r.data_fim, 1, 4) = ? "
        "GROUP BY ge.nome ORDER BY n DESC, ge.nome LIMIT 1"));
    g.addBindValue(anoStr);
    if (g.exec() && g.next()) e.generoMaisLido = g.value(0).toString();

    QSqlQuery a(db());
    a.prepare(QStringLiteral(
        "SELECT COUNT(*) FROM leitura WHERE status = 'ABANDONADO' AND substr(data_inicio, 1, 4) = ?"));
    a.addBindValue(anoStr);
    if (a.exec() && a.next()) e.livrosAbandonados = a.value(0).toInt();

    QSqlQuery n(QStringLiteral("SELECT COUNT(*) FROM leitura WHERE status = 'LENDO'"), db());
    if (n.next()) e.livrosLendo = n.value(0).toInt();
    return e;
}

QList<QPair<int, int>> EstatisticaRepository::lidosPorMes(int ano) const { BN_TRACE("EstatisticaRepository::lidosPorMes");
    QList<QPair<int, int>> out;
    for (int m = 1; m <= 12; ++m) out << qMakePair(m, 0);
    QSqlQuery q(db());
    q.prepare(QStringLiteral(
        "SELECT CAST(substr(data_fim, 6, 2) AS INTEGER) AS mes, COUNT(*) FROM leitura "
        "WHERE status = 'LIDO' AND substr(data_fim, 1, 4) = ? GROUP BY mes"));
    q.addBindValue(QString::number(ano));
    if (q.exec()) {
        while (q.next()) {
            const int m = q.value(0).toInt();
            if (m >= 1 && m <= 12) out[m - 1].second = q.value(1).toInt();
        }
    }
    return out;
}

QList<LinhaHistorico> EstatisticaRepository::historicoConcluidos() const { BN_TRACE("EstatisticaRepository::historicoConcluidos");
    QList<LinhaHistorico> out;
    QSqlQuery q(QStringLiteral(
                    "SELECT l.titulo, a.nome, r.data_inicio, r.data_fim, r.nota, l.total_paginas "
                    "FROM leitura r JOIN livro l ON l.id = r.id_livro JOIN autor a ON a.id = l.id_autor "
                    "WHERE r.status = 'LIDO' ORDER BY r.data_fim DESC, l.titulo"),
                db());
    while (q.next()) {
        LinhaHistorico h;
        h.titulo = q.value(0).toString();
        h.autor = q.value(1).toString();
        h.dataInicio = q.value(2).toString();
        h.dataFim = q.value(3).toString();
        h.nota = q.value(4).isNull() ? 0 : q.value(4).toInt();
        h.paginas = q.value(5).toInt();
        out << h;
    }
    return out;
}

}  // namespace booknest
