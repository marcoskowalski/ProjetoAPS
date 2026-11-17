#pragma once
#include <QMainWindow>

#include "db/LeituraRepository.h"
#include "db/LivroRepository.h"

class QSqlTableModel;
class QTableView;
class QLineEdit;
class QComboBox;
class QLabel;

namespace booknest {

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void aplicarFiltro();        // UC005: model->setFilter() a cada alteração do texto
    void cadastrar();            // UC001
    void editar();               // UC002
    void remover();              // UC003
    void registrarProgresso();   // UC004
    void avaliar();              // RF005
    void alterarStatus(booknest::core::Status novo);
    void abrirListas();          // RF007
    void abrirEstatisticas();    // RF008
    void buscarNaWeb();          // Google Books
    void verDetalhes();

private:
    void montarInterface();
    void montarAcoes();
    void recarregar();
    void recarregarGeneros();
    qint64 livroSelecionado(bool avisar = true);
    void mensagem(const QString& texto);

    LivroRepository m_livros;
    LeituraRepository m_leituras;
    QSqlTableModel* m_model = nullptr;
    QTableView* m_view = nullptr;
    QLineEdit* m_busca = nullptr;
    QComboBox* m_filtroStatus = nullptr;
    QComboBox* m_filtroGenero = nullptr;
    QLabel* m_contador = nullptr;
};

}  // namespace booknest
