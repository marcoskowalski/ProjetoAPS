

#ifndef BOOKNEST_BOOKS_H
#define BOOKNEST_BOOKS_H
#include <QString>


class Livro {
    private:
        int id;
        QString autor;
        QString titulo;
        QString genero;
        int numPaginas;
        int anoPublicação;
        QString statusLeitura;

    public:
    ~Livro();
    Livro();
    Livro(int id, QString titulo, QString autor, QString genero, int paginasTotais, int anoPublicacao, QString status);

    int getId() const;
    QString getTitulo() const;
    QString getAutor() const;
    QString getGenero() const;
    int getPaginasTotais() const;
    int getAnoPublicacao() const;
    QString getStatus() const;

    void setId(int id);
    void setTitulo(const QString &titulo);
    void setAutor(const QString &autor);
    void setGenero(const QString &genero);
    void setPaginasTotais(int paginasTotais);
    void setAnoPublicacao(int anoPublicacao);
    void setStatus(const QString &status);



};


#endif