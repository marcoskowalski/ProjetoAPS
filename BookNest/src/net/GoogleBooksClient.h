#pragma once
#include <QList>
#include <QNetworkAccessManager>
#include <QObject>

#include "domain/Models.h"

namespace booknest {

// Busca livros na internet. Tenta primeiro a Google Books API; se ela falhar (ex.: erro 429 por
// excesso de requisições sem chave) ou não devolver nada, usa a Open Library (gratuita, sem chave).
// Opcional: defina a variável de ambiente BOOKNEST_GOOGLE_API_KEY para usar uma chave do Google.
// Funciona apenas online; offline emite failed() com mensagem amigável.
class GoogleBooksClient : public QObject {
    Q_OBJECT
public:
    explicit GoogleBooksClient(QObject* parent = nullptr);
    void buscar(const QString& termo);

    // Converte o JSON da API em livros (separado para facilitar testes).
    static QList<Livro> parseResposta(const QByteArray& json);
    static QList<Livro> parseOpenLibrary(const QByteArray& json);

signals:
    void resultados(const QList<booknest::Livro>& livros);
    void failed(const QString& mensagem);

private:
    void buscarGoogle();
    void buscarOpenLibrary(const QString& erroGoogle);

    QNetworkAccessManager m_net;
    QString m_termo;
};

}  // namespace booknest
