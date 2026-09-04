#include "widget.h"
#include "ui_widget.h"
#include "src/service/login.h"
#include "src/ui/mainwindow.h"

#include <QMessageBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect(ui->btn_login, &QPushButton::clicked, this, &Widget::onLoginClicked);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::onLoginClicked()
{
    QString username = ui->lineEdit_username->text().trimmed();
    QString password = ui->lineEdit_password->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入账号和密码");
        return;
    }

    auto admin = LoginService::login(username, password);
    if (admin.has_value()) {
        MainWindow *mainWin = new MainWindow(admin->realName());
        mainWin->show();
        this->close();
    } else {
        QMessageBox::critical(this, "错误", "账号或密码错误");
    }
}
