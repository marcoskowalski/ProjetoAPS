// Testes das regras de negócio (sem Qt). Executar: ctest  ou  ./test_core
#include <cstdio>
#include <cstdlib>

#include "core/ReadingRules.h"

using namespace booknest::core;

static int failures = 0;
#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::printf("FALHOU %s:%d  %s\n", __FILE__, __LINE__, #cond);        \
            ++failures;                                                          \
        }                                                                        \
    } while (0)

int main() {
    // RN001 / RN002
    CHECK(!validateBook("", "A", "G", 10, 2000, "").ok);
    CHECK(!validateBook("   ", "A", "G", 10, 2000, "").ok);
    CHECK(!validateBook("T", "A", "G", 0, 2000, "").ok);
    CHECK(!validateBook("T", "A", "G", -5, 2000, "").ok);
    CHECK(!validateBook("T", "", "G", 10, 2000, "").ok);
    CHECK(!validateBook("T", "A", "", 10, 2000, "").ok);
    CHECK(validateBook("T", "A", "G", 10, 0, "").ok);          // ano não informado
    CHECK(!validateBook("T", "A", "G", 10, 3000, "").ok);
    CHECK(validateBook("T", "A", "G", 10, 1999, "").ok);

    // ISBN
    CHECK(isValidIsbn("978-0-306-40615-7"));
    CHECK(isValidIsbn("9780306406157"));
    CHECK(isValidIsbn("0-306-40615-2"));
    CHECK(isValidIsbn("080442957X"));
    CHECK(!isValidIsbn("9780306406158"));
    CHECK(!isValidIsbn("123"));
    CHECK(!validateBook("T", "A", "G", 10, 1999, "123").ok);
    CHECK(validateBook("T", "A", "G", 10, 1999, "9780306406157").ok);

    // porcentagem
    CHECK(progressPercent(50, 200) == 25.0);
    CHECK(progressPercent(1, 3) == 33.3);
    CHECK(progressPercent(5, 0) == 0.0);
    CHECK(progressPercent(300, 200) == 100.0);

    // UC004 / RN005
    auto p = applyProgress(Status::Lendo, 10, 60, 200);
    CHECK(p.result.ok && p.pagesRead == 50 && p.percent == 30.0 && !p.finished &&
          p.newStatus == Status::Lendo);
    p = applyProgress(Status::Lendo, 10, 200, 200);                // FA01
    CHECK(p.result.ok && p.finished && p.newStatus == Status::Lido && p.percent == 100.0);
    p = applyProgress(Status::Lendo, 10, 201, 200);                // FE01
    CHECK(!p.result.ok);
    p = applyProgress(Status::Lendo, 10, 5, 200);                  // RN005
    CHECK(!p.result.ok);
    p = applyProgress(Status::QueroLer, 0, 5, 200);                // pré-condição
    CHECK(!p.result.ok);
    p = applyProgress(Status::Lendo, 10, 10, 200);                 // mesma página: permitido
    CHECK(p.result.ok && p.pagesRead == 0);

    // troca de status
    auto s = applyStatusChange(Status::QueroLer, Status::Lendo, 0, 300, false);
    CHECK(s.result.ok && s.setStartDate && !s.setEndDate && s.newPage == 0);
    s = applyStatusChange(Status::Lendo, Status::Lido, 120, 300, true);
    CHECK(s.newPage == 300 && s.setEndDate && !s.setStartDate);
    s = applyStatusChange(Status::Lido, Status::Lendo, 300, 300, true);
    CHECK(s.clearEndDate && s.newPage == 300);

    s = applyStatusChange(Status::Lendo, Status::QueroLer, 50, 300, true);
    CHECK(s.newPage == 0);

    // avaliação
    CHECK(validateRating(Status::Lido, 5).ok);
    CHECK(!validateRating(Status::Lido, 0).ok);
    CHECK(!validateRating(Status::Lido, 6).ok);
    CHECK(!validateRating(Status::Lendo, 3).ok);

    // status <-> código
    for (auto st : {Status::QueroLer, Status::Lendo, Status::Lido, Status::Abandonado})
        CHECK(statusFromCode(statusCode(st)) == st);
    CHECK(!statusFromCode("XYZ").has_value());

    if (failures == 0) std::printf("OK: todos os testes passaram\n");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
