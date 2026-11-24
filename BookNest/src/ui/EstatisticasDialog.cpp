#include "EstatisticasDialog.h"
#include "core/Trace.h"

#include <QComboBox>
#include <QDate>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QTabWidget>
#include <QVBoxLayout>

namespace booknest {

namespace {
QTableWidgetItem* item(const QString& t, Qt::Alignment a = Qt::AlignLeft | Qt::AlignVCenter) {
    auto* i = new QTableWidgetItem(t);
    i->setTextAlignment(a);
    i->setFlags(i->flags() & ~Qt::ItemIsEditable);
    return i;
}
QString isoParaBr(const QString& iso) {
    const QDate d = QDate::fromString(iso, QStringLiteral("yyyy-MM-dd"));
    return d.isValid() ? d.toString(QStringLiteral("dd/MM/yyyy")) : QStringLiteral("—");
}
}  // namespace

EstatisticasDialog::EstatisticasDialog(QWidget* parent) : QDialog(parent) { BN_TRACE("EstatisticasDialog::EstatisticasDialog");
    setWindowTitle(tr("Histórico e estatísticas de leitura"));
    resize(760, 520);

    // --- aba 1: estatísticas anuais
    m_ano = new QComboBox;
    auto anos = m_repo.anosComLeitura();
    if (!anos.contains(QDate::currentDate().year())) anos.prepend(QDate::currentDate().year());
    for (int a : anos) m_ano->addItem(QString::number(a), a);
    m_resumo = new QLabel;
    m_resumo->setTextFormat(Qt::RichText);
    m_resumo->setWordWrap(true);

    m_meses = new QTableWidget(12, 3);
    m_meses->setHorizontalHeaderLabels({tr("Mês"), tr("Livros concluídos"), QString()});
    m_meses->verticalHeader()->hide();
    m_meses->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_meses->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_meses->setSelectionMode(QAbstractItemView::NoSelection);

    auto* aba1 = new QWidget;
    auto* l1 = new QVBoxLayout(aba1);
    l1->addWidget(m_ano);
    l1->addWidget(m_resumo);
    l1->addWidget(m_meses);

    // --- aba 2: histórico
    m_historico = new QTableWidget(0, 6);
    m_historico->setHorizontalHeaderLabels(
        {tr("Título"), tr("Autor"), tr("Início"), tr("Término"), tr("Nota"), tr("Páginas")});
    m_historico->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_historico->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_historico->verticalHeader()->hide();
    m_historico->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historico->setSelectionBehavior(QAbstractItemView::SelectRows);

    auto* tabs = new QTabWidget;
    tabs->addTab(aba1, tr("Estatísticas anuais"));
    tabs->addTab(m_historico, tr("Histórico de leituras"));
    auto* lay = new QVBoxLayout(this);
    lay->addWidget(tabs);

    connect(m_ano, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { carregarAno(); });
    carregarAno();
    carregarHistorico();
}

void EstatisticasDialog::carregarAno() { BN_TRACE("EstatisticasDialog::carregarAno");
    const int ano = m_ano->currentData().toInt();
    const auto e = m_repo.anual(ano);
    QString html = tr("<p><b>Livros lidos em %1:</b> %2<br>"
                      "<b>Páginas lidas:</b> %3<br>")
                       .arg(ano).arg(e.livrosLidos).arg(e.paginasLidas);
    html += e.notaMedia > 0 ? tr("<b>Nota média:</b> %1 ★<br>").arg(e.notaMedia, 0, 'f', 1)
                            : tr("<b>Nota média:</b> —<br>");
    html += tr("<b>Gênero mais lido:</b> %1<br>")
                .arg(e.generoMaisLido.isEmpty() ? QStringLiteral("—") : e.generoMaisLido);
    html += tr("<b>Abandonados (iniciados no ano):</b> %1 &nbsp; <b>Lendo agora:</b> %2</p>")
                .arg(e.livrosAbandonados).arg(e.livrosLendo);
    m_resumo->setText(html);

    const auto meses = m_repo.lidosPorMes(ano);
    int max = 1;
    for (const auto& p : meses) max = qMax(max, p.second);
    static const char* nomes[] = {"Janeiro", "Fevereiro", "Março", "Abril", "Maio", "Junho", "Julho",
                                  "Agosto", "Setembro", "Outubro", "Novembro", "Dezembro"};
    for (int i = 0; i < meses.size(); ++i) {
        m_meses->setItem(i, 0, item(QString::fromUtf8(nomes[i])));
        m_meses->setItem(i, 1, item(QString::number(meses[i].second), Qt::AlignCenter));
        const int blocos = meses[i].second * 30 / max;
        m_meses->setItem(i, 2, item(QString(blocos, QChar(0x2588))));  // barra de texto
    }
}

void EstatisticasDialog::carregarHistorico() { BN_TRACE("EstatisticasDialog::carregarHistorico");
    const auto linhas = m_repo.historicoConcluidos();
    m_historico->setRowCount(linhas.size());
    for (int i = 0; i < linhas.size(); ++i) {
        const auto& h = linhas[i];
        m_historico->setItem(i, 0, item(h.titulo));
        m_historico->setItem(i, 1, item(h.autor));
        m_historico->setItem(i, 2, item(isoParaBr(h.dataInicio), Qt::AlignCenter));
        m_historico->setItem(i, 3, item(isoParaBr(h.dataFim), Qt::AlignCenter));
        m_historico->setItem(i, 4,
                             item(h.nota > 0 ? QString(h.nota, QChar(0x2605)) : QStringLiteral("—"),
                                  Qt::AlignCenter));
        m_historico->setItem(i, 5, item(QString::number(h.paginas), Qt::AlignCenter));
    }
}

}  // namespace booknest
