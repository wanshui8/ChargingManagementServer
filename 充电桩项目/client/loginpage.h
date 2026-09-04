#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QLabel;
class TcpClient;
class MainWindow;

// 登录页：手机号免密登录（首次自动注册），支持配置服务器地址
class LoginPage : public QWidget
{
    Q_OBJECT
public:
    explicit LoginPage(TcpClient *client, MainWindow *mainWindow, QWidget *parent = nullptr);

private slots:
    void onLoginClicked();
    void onConnected();
    void onMessageReceived(quint16 msgId, const QJsonObject &payload);
    void onError(const QString &error);

private:
    void sendLogin();

    TcpClient *m_client;
    MainWindow *m_mainWindow;

    QLineEdit   *m_phoneEdit;
    QLineEdit   *m_hostEdit;
    QLineEdit   *m_portEdit;
    QPushButton *m_loginButton;
    QLabel      *m_statusLabel;
};

#endif // LOGINPAGE_H