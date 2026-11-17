#pragma once
#include <QList>
#include <QPair>
#include <QString>

namespace booknest {

struct EstatisticaAnual {
    int ano = 0;
    int livrosLidos = 0;
    int paginasLidas = 0;       // soma das páginas dos livros concluídos no ano
    double notaMedia = 0.0;     // 0 se nenhum livro avaliado
    QString generoMaisLido;     // vazio se nenhum
    int livrosAbandonados = 0;  // abandonados com início no ano
    int livrosLendo = 0;        // em andamento (independe do ano)
};

struct LinhaHistorico {
    QString titulo;
    QString autor;
    QString dataInicio;
    QString dataFim;
    int nota = 0;
    int paginas = 0;
};

// RF008: histórico completo de leituras e estatísticas anuais.
class EstatisticaRepository {
public:
    QList<int> anosComLeitura() const;  // anos em que houve conclusão, do mais recente ao mais antigo
    EstatisticaAnual anual(int ano) const;
    QList<QPair<int, int>> lidosPorMes(int ano) const;  // (mês 1..12, quantidade)
    QList<LinhaHistorico> historicoConcluidos() const;
};

}  // namespace booknest
