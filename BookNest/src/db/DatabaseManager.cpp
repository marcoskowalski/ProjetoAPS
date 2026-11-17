#include "DatabaseManager.h"

#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

namespace booknest {

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

QSqlDatabase DatabaseManager::database() const {
    return QSqlDatabase::database(kConnection);
}

bool DatabaseManager::runScript(const QString& resource, QString* error) {
    QFile f(resource);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) *error = QStringLiteral("Não foi possível ler o recurso %1").arg(resource);
        return false;
    }
    const QString text = QString::fromUtf8(f.readAll());

    // remove comentários de linha e divide em instruções por ';'
    QStringList lines;
    for (const QString& line : text.split('\n')) {
        if (line.trimmed().startsWith(QStringLiteral("--"))) continue;
        lines << line;
    }
    const QStringList statements = lines.join('\n').split(';');

    QSqlDatabase db = database();
    for (const QString& raw : statements) {
        const QString sql = raw.trimmed();
        if (sql.isEmpty()) continue;
        QSqlQuery q(db);
        if (!q.exec(sql)) {
            if (error)
                *error = QStringLiteral("Erro ao executar %1: %2\n%3")
                             .arg(resource, q.lastError().text(), sql.left(120));
            return false;
        }
    }
    return true;
}

bool DatabaseManager::open(const QString& path, QString* error) {
    m_path = path;
    QSqlDatabase db = QSqlDatabase::contains(kConnection)
                          ? QSqlDatabase::database(kConnection)
                          : QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), kConnection);
    db.setDatabaseName(path);
    if (!db.open()) {
        if (error) *error = db.lastError().text();
        return false;
    }
    QSqlQuery(db).exec(QStringLiteral("PRAGMA foreign_keys = ON"));
    QSqlQuery(db).exec(QStringLiteral("PRAGMA journal_mode = WAL"));

    if (!runScript(QStringLiteral(":/schema.sql"), error)) return false;

    // user_version = 0 -> banco novo: carrega catálogo inicial uma única vez
    QSqlQuery v(db);
    int version = 0;
    if (v.exec(QStringLiteral("PRAGMA user_version")) && v.next()) version = v.value(0).toInt();
    if (version < 1) {
        if (!db.transaction()) {
            if (error) *error = db.lastError().text();
            return false;
        }
        if (!runScript(QStringLiteral(":/seed.sql"), error)) {
            db.rollback();
            return false;
        }
        db.commit();
        QSqlQuery(db).exec(QStringLiteral("PRAGMA user_version = 1"));
    }
    return true;
}

}  // namespace booknest
