#pragma once
#include <QSqlDatabase>
#include <QString>

namespace booknest {

// Abre o arquivo SQLite, cria o esquema (resources/schema.sql) e, na primeira execução,
// carrega o catálogo inicial de livros (resources/seed.sql).
class DatabaseManager {
public:
    static DatabaseManager& instance();

    bool open(const QString& path, QString* error);
    QSqlDatabase database() const;
    QString path() const { return m_path; }

private:
    DatabaseManager() = default;
    bool runScript(const QString& resource, QString* error);

    QString m_path;
    static constexpr const char* kConnection = "booknest";
};

}  // namespace booknest
