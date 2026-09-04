#ifndef ADMINLOGINWIDGET_H
#define ADMINLOGINWIDGET_H

#include <QDialog>

class QLineEdit;
class QPushButton;
class QLabel;
class ServerService;

// 管理员登录对话框
class AdminLoginWidget : public QDialog
{
    Q_OBJECT
public:
    explicit AdminLoginWidget(ServerService *service, QWidget *parent = nullptr);

private slots:
    void onLoginClicked();
    void onLoginResult(bool ok);

private:
    ServerService *m_service;

    QLineEdit     *m_usernameEdit;
    QLineEdit     *m_passwordEdit;
    QPushButton   *m_loginButton;
    QLabel        *m_hintLabel;
};

#endif // ADMINLOGINWIDGET_H