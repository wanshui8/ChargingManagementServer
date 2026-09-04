#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(const QString& username, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->label_welcome->setText("Hello, " + username);
}

MainWindow::~MainWindow()
{
    delete ui;
}
