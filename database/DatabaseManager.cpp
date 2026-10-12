#include "DatabaseManager.h"



DatabaseManager::DatabaseManager() {
    db = QSqlDatabase::addDatabase("QSQLITE");
}
DatabaseManager::~DatabaseManager() {
    fecharConexao();
}
DatabaseManager &DatabaseManager::instance() {
    static DatabaseManager db;
    return db;
}
bool DatabaseManager::isOpen() {
    return db.open();
}
bool DatabaseManager::abrirConexao(const QString &caminhoDb) {
    if (db.isOpen()) {
        return true;
    }

    db.setDatabaseName(caminhoDb);

    if (!db.open()) {
        qDebug()<<"Cannot open database"<< db.lastError().text();
        return false;
    }
    qDebug()<<"Connection sucessed";
    return inicializarTabelas();
}
bool DatabaseManager::inicializarTabelas() {
    QSqlQuery query;

    QString sqlLivros = R"(
        CREATE TABLE IF NOT EXISTS livros (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            titulo TEXT NOT NULL,
            autor TEXT NOT NULL,
            genero TEXT,
            paginas_totais INTEGER,
            ano_publicacao INTEGER,
            status TEXT DEFAULT 'Quero Ler'
        )
    )";

    if (!query.exec(sqlLivros)) {
        qDebug() << "Erro ao criar tabela 'livros':" << query.lastError().text();
        return false;
    }

    return true;
}
void DatabaseManager::fecharConexao() {
    if (db.isOpen()) {
        db.close();
        qDebug()<<"Closing database...";
    }
}
QSqlDatabase &DatabaseManager::getDatabase() {
    return db;
}

bool DatabaseManager::livroExiste(const QString &titulo) {
    QSqlQuery query;
    // Mudamos 'livro' para 'titulo'
    query.prepare("SELECT COUNT(*) FROM livros WHERE titulo = ?");
    query.addBindValue(titulo);

    if (!query.exec()) {
        qDebug() << "Erro ao executar query livroExiste:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        int count = query.value(0).toInt();
        qDebug() << "Procurando por:" << titulo << "| Registos encontrados no DB:" << count;
        return count > 0;
    }

    return false;
}