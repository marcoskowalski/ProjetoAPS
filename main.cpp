#include <iostream>

#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include "database/databasemanager.h"
#include "database/LivroRepo.h"
#include "models/Livro.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 1. Inicializa o Banco de Dados
    if (!DatabaseManager::instance().abrirConexao("booknest.db")) {
        qDebug() << "nao foi possivel abrir o db";
        return -1;
    }

    // 2. Teste Prático: Inserir um livro de teste


    Livro novoLivro(2, "bruddas in paris", "J.R.R. Tolkien", "Fantasia", 1200, 1954, "Quero Ler");

    QString tituloLivro = (novoLivro.getTitulo()); // ou .titulo() dependendo do seu getter

    if (DatabaseManager::instance().livroExiste(tituloLivro)) {
        qDebug() << "Livro ja existe";
    }
    else {
        if (LivroRepo::inserir(novoLivro)) {
            qDebug() << "Livro inserido com sucesso via código!";
        }
    }



    // 4. Inicializa a Janela Principal (caso queira abrir o app normal)
    MainWindow w;
    w.show();



    return a.exec();
}