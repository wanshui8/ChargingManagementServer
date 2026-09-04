#include "adminloginwidget.h"
#include "serverservice.h"

#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFrame>

AdminLoginWidget::AdminLoginWidget(ServerService *service, QWidget *parent)
    : QDialog(parent)
    , m_service(service)
{
    setWindowTitle("管理员登录");
    setFixedSize(380, 240);

    QLabel *titleLabel = new QLabel("充电桩管理平台 · 服务器端");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    m_usernameEdit = new QLineEdit;
    m_usernameEdit->setPlaceholderText("请输入账号");
    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setPlaceholderText("请输入密码");
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    m_loginButton = new QPushButton("登 录");
    m_loginButton->setDefault(true);

    m_hintLabel = new QLabel("默认账号：admin   默认密码：123456");
    m_hintLabel->setStyleSheet("color: gray;");
    m_hintLabel->setAlignment(Qt::AlignCenter);

    QFormLayout *form = new QFormLayout;
    form->addRow("账 号：", m_usernameEdit);
    form->addRow("密 码：", m_passwordEdit);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(16);
    mainLayout->addLayout(form);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(m_loginButton);
    mainLayout->addWidget(m_hintLabel);

    connect(m_loginButton, &QPushButton::clicked, this, &AdminLoginWidget::onLoginClicked);
    connect(m_service, &ServerService::adminLoginResult,
            this, &AdminLoginWidget::onLoginResult);
}

void AdminLoginWidget::onLoginClicked()
{
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入账号和密码");
        return;
    }
    m_loginButton->setEnabled(false);
    // 异步验证（数据库在业务线程）
    m_service->asyncAdminLogin(username, password);
}

void AdminLoginWidget::onLoginResult(bool ok)
{
    m_loginButton->setEnabled(true);
    if (ok) {
        accept();
    } else {
        QMessageBox::warning(this, "登录失败", "账号或密码错误");
    }
}