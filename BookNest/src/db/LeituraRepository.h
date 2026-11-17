#pragma once
#include <optional>

#include "core/ReadingRules.h"
#include "domain/Models.h"

namespace booknest {

// Acompanhamento de leitura: status, progresso (UC004), nota e resenha (RF005), histórico.
class LeituraRepository {
public:
    std::optional<Leitura> porLivro(qint64 idLivro) const;
    bool alterarStatus(qint64 idLivro, Status novo, QString* erro);
    // Aplica as regras de core::applyProgress e grava o resultado.
    bool registrarProgresso(qint64 idLivro, int novaPagina, core::ProgressOutcome* resultado,
                            QString* erro);
    bool avaliar(qint64 idLivro, int nota, const QString& resenha, QString* erro);
    QList<RegistroProgresso> historico(qint64 idLivro) const;
};

}  // namespace booknest
