#include "loginpage.h"
#include "tcpclient.h"
#include "mainwindow.h"

#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonObject>

LoginPage::LoginPage(TcpClient *client, MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent)
    , m_client(client)
    , m_mainWindow(mainWindow)
{
    QLabel *titleLabel = new QLabel("电动充电用户端");
    QFont f = titleLabel->font();
    f.setPointSize(18);
    f.setBold(true);
    titleLabel->setFont(f);
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *subLabel = new QLabel("输入手机号，免密登录 / 自动注册");
    subLabel->setAlignment(Qt::AlignCenter);
    subLabel->setStyleSheet("color: gray;");

    m_phoneEdit = new QLineEdit;
    m_phoneEdit->setPlaceholderText("请输入 11 位手机号");
    m_phoneEdit->setMaxLength(11);

    m_hostEdit = new QLineEdit("127.0.0.1");
    m_portEdit = new QLineEdit("9527");

    QHBoxLayout *serverLayout = new QHBoxLayout;
    QLabel *hostLabel = new QLabel("服务器:");
    QLabel *portLabel = new QLabel("端口:");
    serverLayout->addWidget(hostLabel);
    serverLayout->addWidget(m_hostEdit);
    serverLayout->addWidget(portLabel);
    serverLayout->addWidget(m_portEdit);

    m_loginButton = new QPushButton("登录 / 注册");
    m_loginButton->setMinimumHeight(42);

    m_statusLabel = new QLabel("请先连接服务器");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: orange;");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 60, 40, 40);
    layout->addWidget(titleLabel);
    layout->addWidget(subLabel);
    layout->addSpacing(30);
    layout->addWidget(m_phoneEdit);
    layout->addSpacing(10);
    layout->addLayout(serverLayout);
    layout->addSpacing(20);
    layout->addWidget(m_loginButton);
    layout->addSpacing(10);
    layout->addWidget(m_statusLabel);
    layout->addStretch();

    connect(m_loginButton, &QPushButton::clicked, this, &LoginPage::onLoginClicked);
    connect(m_client, &TcpClient::connected, this, &LoginPage::onConnected);
    connect(m_client, &TcpClient::messageReceived, this, &LoginPage::onMessageReceived);
    connect(m_client, &TcpClient::errorOccurred, this, &LoginPage::onError);
}

void LoginPage::onLoginClicked()
{
    QString phone = m_phoneEdit->text().trimmed();
    if (phone.size() != 11) {
        m_statusLabel->setText("请输入 11 位手机号");
        return;
    }

    m_loginButton->setEnabled(false);
    m_statusLabel->setText("正在连接服务器...");

    QString host = m_hostEdit->text().trimmed();
    quint16 port = static_cast<quint16>(m_portEdit->text().toUInt());

    if (!m_client->isConnected()) {
        m_client->connectToServer(host, port);
        // 连接成功后 onConnected 会触发 sendLogin，这里暂存手机号
        m_phoneEdit->setProperty("pending_phone", phone);
    } else {
        sendLogin();
    }
}

void LoginPage::onConnected()
{
    m_statusLabel->setText("已连接服务器，正在登录...");
    sendLogin();
}

void LoginPage::sendLogin()
{
    QString phone = m_phoneEdit->property("pending_phone").toString();
    if (phone.isEmpty()) {
        phone = m_phoneEdit->text().trimmed();
    }
    QJsonObject payload;
    payload["phone"] = phone;
    m_client->send(Protocol::MsgId::REQ_LOGIN, payload);
}

void LoginPage::onMessageReceived(quint16 msgId, const QJsonObject &payload)
{
    if (msgId != Protocol::MsgId::RES_LOGIN) {
        return;
    }

    m_loginButton->setEnabled(true);

    int code = payload.value("code").toInt();
    if (code == Protocol::ErrorCode::OK) {
        QJsonObject userObj = payload.value("user").toObject();
        UserInfo user;
        user.id = userObj.value("id").toInt();
        user.phone = userObj.value("phone").toString();
        user.nickname = userObj.value("nickname").toString();
        user.avatar = userObj.value("avatar").toString();
        user.balance = userObj.value("balance").toDouble();
        user.status = userObj.value("status").toString();
        user.createTime = userObj.value("create_time").toString();
        m_client->setCurrentUser(user);

        m_statusLabel->setText("登录成功");
        m_mainWindow->switchTo(MainWindow::PageStationList);
    } else if (code == Protocol::ErrorCode::USER_FROZEN) {
        m_statusLabel->setText("账号已被冻结，请联系管理员");
    } else {
        m_statusLabel->setText("登录失败，错误码: " + QString::number(code));
    }
}

void LoginPage::onError(const QString &error)
{
    m_loginButton->setEnabled(true);
    m_statusLabel->setText("连接失败: " + error);
}