#pragma once
// Regras de negócio do BookNest em C++20 puro (sem Qt), para serem testáveis
// isoladamente. Cada função referencia a regra do plano de projeto (RNxxx).

#include <optional>
#include <string>
#include <string_view>

namespace booknest::core {

enum class Status { QueroLer, Lendo, Lido, Abandonado };

std::string statusCode(Status s);                          // "QUERO_LER", ...
std::string statusLabel(Status s);                         // "Quero Ler", ...
std::optional<Status> statusFromCode(std::string_view c);  // inverso de statusCode

struct Result {
    bool ok = true;
    std::string message;  // mensagem pronta para exibir em QMessageBox
    static Result success() { return {true, {}}; }
    static Result fail(std::string m) { return {false, std::move(m)}; }
};

// RN001 (título obrigatório), RN002 (páginas inteiras > 0).
// Também valida autor/gênero obrigatórios, ano (0 = não informado) e ISBN opcional.
Result validateBook(std::string_view titulo, std::string_view autor, std::string_view genero,
                    int totalPaginas, int anoPublicacao, std::string_view isbn);

// ISBN-10 ou ISBN-13 (hífens/espaços ignorados) com dígito verificador correto.
bool isValidIsbn(std::string_view isbn);

// Porcentagem concluída (0..100, 1 casa decimal de precisão).
double progressPercent(int paginaAtual, int totalPaginas);

struct ProgressOutcome {
    Result result;
    Status newStatus = Status::Lendo;
    int pagesRead = 0;       // páginas lidas neste registro (nova - anterior)
    double percent = 0.0;
    bool finished = false;   // FA01: atingiu o total -> status passa a Lido
};

// UC004 / RN005: precondição status == Lendo; paginaAnterior <= nova <= total.
ProgressOutcome applyProgress(Status atual, int paginaAnterior, int novaPagina, int totalPaginas);

struct StatusChange {
    Result result;
    int newPage = 0;
    bool setStartDate = false;  // true ao entrar em "Lendo" sem data de início
    bool setEndDate = false;    // true ao entrar em "Lido"
    bool clearEndDate = false;  // true ao sair de "Lido"
};

// Troca manual de status. "Lido" completa as páginas; "Quero Ler" zera a página;
// os demais mantêm a página atual.
StatusChange applyStatusChange(Status de, Status para, int paginaAtual, int totalPaginas,
                               bool temDataInicio);

// RF005: nota de 1 a 5 e somente para livros já lidos.
Result validateRating(Status atual, int nota);

}  // namespace booknest::core
