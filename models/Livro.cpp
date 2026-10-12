#include "Livro.h"

#include <filesystem>


Livro::~Livro() = default;

Livro::Livro() : id(0), titulo(""), autor(""), genero(""){}
Livro::Livro(int id, QString titulo, QString autor, QString genero, int paginasTotais, int anoPublicacao, QString status)
    : id(id),
      autor(autor),
      titulo(titulo),
      genero(genero),
      numPaginas(paginasTotais),
      anoPublicação(anoPublicacao),
      statusLeitura(status) {}


int Livro::getId() const {
    return id;
}

QString Livro::getTitulo() const {
    return titulo;
}

QString Livro::getAutor() const {
    return autor;
}

QString Livro::getGenero() const {
    return genero;
}

int Livro::getPaginasTotais() const {
    return numPaginas;
}

int Livro::getAnoPublicacao() const {
    return anoPublicação;
}

QString Livro::getStatus() const {
    return statusLeitura;
}

// Setters
void Livro::setId(int id) {
    this->id = id;
}

void Livro::setTitulo(const QString &titulo) {
    this->titulo = titulo;
}

void Livro::setAutor(const QString &autor) {
    this->autor = autor;
}

void Livro::setGenero(const QString &genero) {
    this->genero = genero;
}

void Livro::setPaginasTotais(int paginasTotais) {
    this->numPaginas = paginasTotais;
}

void Livro::setAnoPublicacao(int anoPublicacao) {
    this->anoPublicação = anoPublicacao;
}

void Livro::setStatus(const QString &status) {
    this->statusLeitura = status;
}