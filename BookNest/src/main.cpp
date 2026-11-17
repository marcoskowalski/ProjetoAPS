#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>

#include "db/DatabaseManager.h"
#include "ui/MainWindow.h"

static const char* kStyle = R"(
QMainWindow, QDialog { background: #faf7f2; }
QToolBar { background: #f1e9dc; border: none; spacing: 6px; padding: 4px; }
QToolButton { padding: 6px 10px; border-radius: 6px; }
QToolButton:hover { background: #e4d6bf; }
QLineEdit, QComboBox, QSpinBox, QPlainTextEdit {
    background: white; border: 1px solid #cdbfa8; border-radius: 6px; padding: 5px; }
QPushButton { background: #8a5a2b; color: white; border: none; border-radius: 6px; padding: 6px 14px; }
QPushButton:hover { background: #a06a34; }
QTableView { background: white; alternate-background-color: #f8f3ea; gridline-color: #eadfcb;
    selection-background-color: #d9b98a; selection-color: black; border: 1px solid #cdbfa8; }
QHeaderView::section { background: #f1e9dc; border: none; border-bottom: 1px solid #cdbfa8; padding: 6px; font-weight: bold; }
)";

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("BookNest"));
    QApplication::setOrganizationName(QStringLiteral("BookNest"));
    app.setStyleSheet(QString::fromUtf8(kStyle));

    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    QString erro;
    if (!booknest::DatabaseManager::instance().open(dir + QStringLiteral("/booknest.db"), &erro)) {
        QMessageBox::critical(nullptr, QStringLiteral("BookNest"),
                              QStringLiteral("Não foi possível abrir o banco de dados:\n%1").arg(erro));
        return 1;
    }

    booknest::MainWindow w;
    w.show();
    return app.exec();
}
