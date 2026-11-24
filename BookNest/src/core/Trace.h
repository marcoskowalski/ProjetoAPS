#pragma once
// Rastreamento opcional de chamadas, para conferir a ordem das mensagens dos diagramas de sequência.
// Desligado por padrão (BN_TRACE não faz nada). Para ligar no CLion:
//   Settings > Build > CMake > CMake options:  -DBOOKNEST_TRACE=ON   (e recarregar o CMake)
// Cada chamada escreve "[TRACE nnn] Classe::metodo" no console e em trace.log (pasta de execução).
#ifdef BOOKNEST_TRACE
#include <cstdio>
namespace booknest::trace {
inline void log(const char* nome) {
    static int n = 0;
    static std::FILE* arquivo = std::fopen("trace.log", "w");  // recomeça a cada execução
    ++n;
    std::fprintf(stderr, "[TRACE %03d] %s\n", n, nome);
    std::fflush(stderr);
    if (arquivo) {
        std::fprintf(arquivo, "%03d %s\n", n, nome);
        std::fflush(arquivo);
    }
}
}  // namespace booknest::trace
#define BN_TRACE(nome) ::booknest::trace::log(nome)
#else
#define BN_TRACE(nome) ((void)0)
#endif
