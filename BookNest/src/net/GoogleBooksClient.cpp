#include "GoogleBooksClient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

#include "core/ReadingRules.h"

namespace booknest {

GoogleBooksClient::GoogleBooksClient(QObject* parent) : QObject(parent) {}

void GoogleBooksClient::buscar(const QString& termo) {
    m_termo = termo.trimmed();
    buscarGoogle();
}

void GoogleBooksClient::buscarGoogle() {
    QUrl url(QStringLiteral("https://www.googleapis.com/books/v1/volumes"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("q"), m_termo);
    query.addQueryItem(QStringLiteral("maxResults"), QStringLiteral("20"));
    query.addQueryItem(QStringLiteral("printType"), QStringLiteral("books"));
    const QString chave = qEnvironmentVariable("BOOKNEST_GOOGLE_API_KEY");
    if (!chave.isEmpty()) query.addQueryItem(QStringLiteral("key"), chave);
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setTransferTimeout(10000);
    QNetworkReply* reply = m_net.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            const QList<Livro> livros = parseResposta(reply->readAll());
            if (!livros.isEmpty()) {
                emit resultados(livros);
                return;
            }
            buscarOpenLibrary(QString());  // Google respondeu, mas sem resultados
            return;
        }
        buscarOpenLibrary(reply->errorString());  // ex.: 429 Too Many Requests
    });
}

void GoogleBooksClient::buscarOpenLibrary(const QString& erroGoogle) {
    QUrl url(QStringLiteral("https://openlibrary.org/search.json"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("q"), m_termo);
    query.addQueryItem(QStringLiteral("limit"), QStringLiteral("20"));
    query.addQueryItem(QStringLiteral("fields"),
                       QStringLiteral("title,author_name,first_publish_year,number_of_pages_median,"
                                      "isbn,subject,cover_i"));
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setTransferTimeout(10000);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("BookNest/1.0 (projeto academico)"));
    QNetworkReply* reply = m_net.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, erroGoogle]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit failed(QStringLiteral("Não foi possível pesquisar livros na internet. Verifique sua "
                                       "conexão e tente novamente em instantes.\n(%1)")
                            .arg(erroGoogle.isEmpty() ? reply->errorString() : erroGoogle));
            return;
        }
        emit resultados(parseOpenLibrary(reply->readAll()));
    });
}

QList<Livro> GoogleBooksClient::parseOpenLibrary(const QByteArray& json) {
    QList<Livro> out;
    const QJsonArray docs = QJsonDocument::fromJson(json).object().value(QStringLiteral("docs")).toArray();
    for (const QJsonValue& v : docs) {
        const QJsonObject d = v.toObject();
        Livro l;
        l.titulo = d.value(QStringLiteral("title")).toString();
        if (l.titulo.isEmpty()) continue;
        const QJsonArray autores = d.value(QStringLiteral("author_name")).toArray();
        l.autor = autores.isEmpty() ? QString() : autores.first().toString();
        const QJsonArray subj = d.value(QStringLiteral("subject")).toArray();
        l.genero = subj.isEmpty() ? QString() : subj.first().toString();
        l.anoPublicacao = d.value(QStringLiteral("first_publish_year")).toInt();
        l.totalPaginas = d.value(QStringLiteral("number_of_pages_median")).toInt();
        for (const QJsonValue& iv : d.value(QStringLiteral("isbn")).toArray()) {
            const QString isbn = iv.toString();
            if (core::isValidIsbn(isbn.toStdString())) {
                l.isbn = isbn;
                if (isbn.size() == 13) break;
            }
        }
        const int capa = d.value(QStringLiteral("cover_i")).toInt();
        if (capa > 0)
            l.capaUrl = QStringLiteral("https://covers.openlibrary.org/b/id/%1-M.jpg").arg(capa);
        l.origem = QStringLiteral("MANUAL");  // o usuário confere e confirma os dados no formulário
        out << l;
    }
    return out;
}

QList<Livro> GoogleBooksClient::parseResposta(const QByteArray& json) {
    QList<Livro> out;
    const QJsonObject root = QJsonDocument::fromJson(json).object();
    const QJsonArray items = root.value(QStringLiteral("items")).toArray();
    for (const QJsonValue& v : items) {
        const QJsonObject info = v.toObject().value(QStringLiteral("volumeInfo")).toObject();
        Livro l;
        l.titulo = info.value(QStringLiteral("title")).toString();
        if (l.titulo.isEmpty()) continue;
        const QString subt = info.value(QStringLiteral("subtitle")).toString();
        if (!subt.isEmpty()) l.titulo += QStringLiteral(": ") + subt;

        QStringList autores;
        for (const QJsonValue& a : info.value(QStringLiteral("authors")).toArray())
            autores << a.toString();
        l.autor = autores.isEmpty() ? QString() : autores.first();

        const QJsonArray cats = info.value(QStringLiteral("categories")).toArray();
        l.genero = cats.isEmpty() ? QString() : cats.first().toString();

        l.totalPaginas = info.value(QStringLiteral("pageCount")).toInt();
        l.anoPublicacao = info.value(QStringLiteral("publishedDate")).toString().left(4).toInt();
        l.sinopse = info.value(QStringLiteral("description")).toString();

        for (const QJsonValue& idv : info.value(QStringLiteral("industryIdentifiers")).toArray()) {
            const QJsonObject id = idv.toObject();
            if (id.value(QStringLiteral("type")).toString() == QStringLiteral("ISBN_13")) {
                l.isbn = id.value(QStringLiteral("identifier")).toString();
                break;
            }
            if (l.isbn.isEmpty()) l.isbn = id.value(QStringLiteral("identifier")).toString();
        }
        l.capaUrl = info.value(QStringLiteral("imageLinks")).toObject()
                        .value(QStringLiteral("thumbnail")).toString();
        l.origem = QStringLiteral("GOOGLE_BOOKS");
        out << l;
    }
    return out;
}

}  // namespace booknest
