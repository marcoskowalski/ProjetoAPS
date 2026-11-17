#include "ReadingRules.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>

namespace booknest::core {

namespace {
std::string trim(std::string_view v) {
    size_t b = 0, e = v.size();
    while (b < e && std::isspace(static_cast<unsigned char>(v[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(v[e - 1]))) --e;
    return std::string(v.substr(b, e - b));
}
}  // namespace

std::string statusCode(Status s) {
    switch (s) {
        case Status::QueroLer: return "QUERO_LER";
        case Status::Lendo: return "LENDO";
        case Status::Lido: return "LIDO";
        case Status::Abandonado: return "ABANDONADO";
    }
    return "QUERO_LER";
}

std::string statusLabel(Status s) {
    switch (s) {
        case Status::QueroLer: return "Quero Ler";
        case Status::Lendo: return "Lendo";
        case Status::Lido: return "Lido";
        case Status::Abandonado: return "Abandonado";
    }
    return "Quero Ler";
}

std::optional<Status> statusFromCode(std::string_view c) {
    if (c == "QUERO_LER") return Status::QueroLer;
    if (c == "LENDO") return Status::Lendo;
    if (c == "LIDO") return Status::Lido;
    if (c == "ABANDONADO") return Status::Abandonado;
    return std::nullopt;
}

bool isValidIsbn(std::string_view isbn) {
    std::string d;
    for (char c : isbn) {
        if (c == '-' || c == ' ') continue;
        d.push_back(c);
    }
    if (d.size() == 10) {
        int sum = 0;
        for (int i = 0; i < 10; ++i) {
            char c = d[i];
            int v;
            if (i == 9 && (c == 'X' || c == 'x')) v = 10;
            else if (std::isdigit(static_cast<unsigned char>(c))) v = c - '0';
            else return false;
            sum += v * (10 - i);
        }
        return sum % 11 == 0;
    }
    if (d.size() == 13) {
        int sum = 0;
        for (int i = 0; i < 13; ++i) {
            if (!std::isdigit(static_cast<unsigned char>(d[i]))) return false;
            sum += (d[i] - '0') * (i % 2 == 0 ? 1 : 3);
        }
        return sum % 10 == 0;
    }
    return false;
}

Result validateBook(std::string_view titulo, std::string_view autor, std::string_view genero,
                    int totalPaginas, int anoPublicacao, std::string_view isbn) {
    if (trim(titulo).empty()) return Result::fail("O título é obrigatório.");            // RN001
    if (trim(autor).empty()) return Result::fail("O autor é obrigatório.");
    if (trim(genero).empty()) return Result::fail("O gênero é obrigatório.");
    if (totalPaginas <= 0)                                                               // RN002
        return Result::fail("O total de páginas deve ser um número inteiro maior que zero.");
    if (anoPublicacao != 0 && (anoPublicacao < 1 || anoPublicacao > 2100))
        return Result::fail("O ano de publicação informado é inválido.");
    if (!trim(isbn).empty() && !isValidIsbn(isbn))
        return Result::fail("O ISBN informado é inválido (use ISBN-10 ou ISBN-13 válido).");
    return Result::success();
}

double progressPercent(int paginaAtual, int totalPaginas) {
    if (totalPaginas <= 0) return 0.0;
    double p = 100.0 * static_cast<double>(paginaAtual) / static_cast<double>(totalPaginas);
    p = std::clamp(p, 0.0, 100.0);
    return std::round(p * 10.0) / 10.0;
}

ProgressOutcome applyProgress(Status atual, int paginaAnterior, int novaPagina, int totalPaginas) {
    ProgressOutcome out;
    out.newStatus = atual;
    if (atual != Status::Lendo) {
        out.result = Result::fail("Só é possível registrar progresso de livros com status \"Lendo\". "
                                  "Inicie a leitura do livro primeiro.");
        return out;
    }
    if (novaPagina > totalPaginas) {                                                     // FE01 do UC004
        out.result = Result::fail("A página informada (" + std::to_string(novaPagina) +
                                  ") é maior que o total de páginas do livro (" +
                                  std::to_string(totalPaginas) + ").");
        return out;
    }
    if (novaPagina < paginaAnterior) {                                                   // RN005
        out.result = Result::fail("A página informada não pode ser menor que a página atual (" +
                                  std::to_string(paginaAnterior) + ").");
        return out;
    }
    if (novaPagina < 0) {
        out.result = Result::fail("A página informada é inválida.");
        return out;
    }
    out.result = Result::success();
    out.pagesRead = novaPagina - paginaAnterior;
    out.percent = progressPercent(novaPagina, totalPaginas);
    if (novaPagina == totalPaginas) {                                                    // FA01
        out.finished = true;
        out.newStatus = Status::Lido;
    }
    return out;
}

StatusChange applyStatusChange(Status de, Status para, int paginaAtual, int totalPaginas,
                               bool temDataInicio) {
    StatusChange c;
    c.result = Result::success();
    c.newPage = paginaAtual;
    if (de == para) return c;
    if (para == Status::Lendo && !temDataInicio) c.setStartDate = true;
    if (para == Status::Lido) {
        c.newPage = totalPaginas;
        c.setEndDate = true;
        if (!temDataInicio) c.setStartDate = true;
    }
    if (para == Status::QueroLer) c.newPage = 0;  // volta ao início da fila
    if (de == Status::Lido && para != Status::Lido) c.clearEndDate = true;
    return c;
}

Result validateRating(Status atual, int nota) {
    if (atual != Status::Lido)
        return Result::fail("Só é possível avaliar livros com status \"Lido\".");
    if (nota < 1 || nota > 5) return Result::fail("A nota deve estar entre 1 e 5 estrelas.");
    return Result::success();
}

}  // namespace booknest::core
