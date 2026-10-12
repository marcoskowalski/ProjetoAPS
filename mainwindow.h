
#ifndef BOOKNEST_MAINWINDOW_H
#define BOOKNEST_MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QHBoxLayout>
#include <QSqlTableModel>
#include <QHeaderView>
#include <QMouseEvent>
#include<QMessageBox>


QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void changeOverlay(QVBoxLayout *vLayout, QHBoxLayout *hLayout);
    void setLayoutV(QVBoxLayout *vLayout, QLabel *label = nullptr, QTableView *tableView = nullptr, QHBoxLayout *hLayout = nullptr);
    void setLayoutH(QHBoxLayout *hLayout, QLineEdit *lineEdit, QPushButton *pushButton1, QPushButton *pushButton2 = nullptr);

    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    QLabel *label;
    QTableView *tableView;
    QPushButton *pushButton1;
    QPushButton *pushButton2;
    QHBoxLayout *hlayout;
    QVBoxLayout *vLayout;
    QLineEdit *lineEdit;
    QWidget *widget;
    QSqlTableModel *model;
};


#endif //BOOKNEST_MAINWINDOW_H