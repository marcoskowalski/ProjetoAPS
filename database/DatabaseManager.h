
#ifndef BOOKNEST_DATABASEMANAGER_H
#define BOOKNEST_DATABASEMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>


class DatabaseManager {
    private:
        DatabaseManager(); // Construtor privado (padrão Singleton)
        ~DatabaseManager();
        QSqlDatabase db;

    public:
        // Garante que só existe uma instância da conexão na aplicação
        static DatabaseManager& instance();

        bool abrirConexao(const QString &caminhoDb = "booknest.db");
        void fecharConexao();
        bool inicializarTabelas();
        bool isOpen();
        static bool livroExiste(const QString &livro);

        QSqlDatabase& getDatabase();
    };

#endif //BOOKNEST_DATABASEMANAGER_H