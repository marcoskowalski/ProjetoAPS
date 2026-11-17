#pragma once
#include <QDialog>

#include "db/EstatisticaRepository.h"

class QComboBox;
class QLabel;
class QTableWidget;

namespace booknest {

// Histórico completo de leituras e estatísticas anuais (RF008).
class EstatisticasDialog : public QDialog {
    Q_OBJECT
public:
    explicit EstatisticasDialog(QWidget* parent = nullptr);

private:
    void carregarAno();
    void carregarHistorico();

    EstatisticaRepository m_repo;
    QComboBox* m_ano;
    QLabel* m_resumo;
    QTableWidget* m_meses;
    QTableWidget* m_historico;
};

}  // namespace booknest
