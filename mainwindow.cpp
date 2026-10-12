#include "mainwindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    label = new QLabel(this);
    label->setText("teste Label");

    lineEdit = new QLineEdit(this);
    pushButton1 = new QPushButton("pesquisar", this);
    pushButton2 = new QPushButton("cadastrar", this);
    tableView = new QTableView(this);


    hlayout = new QHBoxLayout();
    setLayoutH(hlayout, lineEdit, pushButton1, pushButton2);

    vLayout = new QVBoxLayout();
    setLayoutV(vLayout, label, tableView, hlayout);

    widget = new QWidget(this);
    widget->setLayout(vLayout);
    setCentralWidget(widget);

    model = new QSqlTableModel(this);
    model->setTable("livros");
    model->select();
    tableView->setModel(model);

    // Get the horizontal header and set its resize mode
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(this->pushButton1, &QPushButton::clicked, this, [=]() {
            // 1. Pega o texto digitado na barra de pesquisa e remove espaços extras
            QString termoBusca = this->lineEdit->text().trimmed();

            // 2. Se o campo estiver vazio, podemos limpar o filtro e mostrar tudo de novo
            if (termoBusca.isEmpty()) {
                this->model->setFilter(""); // Remove o filtro
                this->model->select();      // Recarrega os dados
                return;
            }

            // 3. Aplica o filtro no QSqlTableModel procurando na coluna 'titulo'
            // O uso de '%%1%%' serve para buscar livros que contenham o texto em qualquer parte do título
            QString filtro = QString("titulo LIKE '%%1%'").arg(termoBusca);
            this->model->setFilter(filtro);

            // 4. Executa a query para atualizar a QTableView visualmente
            this->model->select();

            // (Opcional) Debug para ver o filtro aplicado no console
            qDebug() << "A pesquisar por:" << termoBusca;
        });
    this->showMaximized();
}

MainWindow::~MainWindow() {
    delete ui;
}
 void MainWindow::setLayoutH(QHBoxLayout *hLayout, QLineEdit *lineEdit, QPushButton *pushButton1, QPushButton *pushButton2) {
    hLayout->addWidget(lineEdit);
    hLayout->addWidget(pushButton1);
    if (pushButton2 != nullptr)
        hLayout->addWidget(pushButton2);
}
void MainWindow::setLayoutV(QVBoxLayout *vLayout, QLabel *label, QTableView *tableView, QHBoxLayout *hLayout) {
    if (tableView != nullptr)
        vLayout->addWidget(tableView);
    if (label != nullptr)
        vLayout->addWidget(label);
    if (hLayout != nullptr)
        vLayout->addLayout(hLayout);
}

