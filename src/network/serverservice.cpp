#include "serverservice.h"
#include "src/dao/appuserdao.h"
#include "src/service/passwordhasher.h"
#include "protocol/Request.h"
#include "protocol/Response.h"
#include "protocol/ProtocolTypes.h"

#include <QDebug>
#include <QJsonObject>

ServerService::ServerService(QObject *parent)
    : QObject(parent), m_tcpServer(new TcpServer(this))
{
    connect(m_tcpServer, &TcpServer::messageReceived,
            this, &ServerService::onMessageReceived);
}

ServerService::~ServerService()
{
    stop();
}

bool ServerService::start(quint16 port)
{
    if (m_tcpServer->startListening(port)) {
        qDebug() << "TCP服务器启动，监听端口:" << port;
        return true;
    }
    qDebug() << "TCP服务器启动失败";
    return false;
}

void ServerService::stop()
{
    m_tcpServer->stopListening();
}

void ServerService::onMessageReceived(QTcpSocket *client, const Message &msg)
{
    qDebug() << "收到消息 OpCode:" << static_cast<int>(msg.opCode())
             << "RequestId:" << msg.requestId();

    switch (msg.opCode()) {
    case Protocol::OpCode::Login:
        handleLogin(client, msg);
        break;
    case Protocol::OpCode::Register:
        handleRegister(client, msg);
        break;
    default:
    {
        Message resp = Response::error(msg.requestId(), msg.opCode(),
                                       Protocol::StatusCode::BadRequest,
                                       "未实现的操作");
        m_tcpServer->sendToClient(client, resp);
        break;
    }
    }
}

void ServerService::handleLogin(QTcpSocket *client, const Message &msg)
{
    QJsonObject payload = msg.payload();
    QString phone = payload["phone"].toString();
    QString password = payload["password"].toString();

    qDebug() << "登录请求 手机号:" << phone;

    auto userOpt = AppUserDao::findByPhone(phone);
    if (!userOpt.has_value()) {
        Message resp = Response::error(msg.requestId(), Protocol::OpCode::Login,
                                       Protocol::StatusCode::Unauthorized,
                                       "用户不存在");
        m_tcpServer->sendToClient(client, resp);
        return;
    }

    const AppUser &user = userOpt.value();
    if (!PasswordHasher::verify(password, user.passwordHash())) {
        Message resp = Response::error(msg.requestId(), Protocol::OpCode::Login,
                                       Protocol::StatusCode::Unauthorized,
                                       "密码错误");
        m_tcpServer->sendToClient(client, resp);
        return;
    }

    if (user.status() != 1) {
        Message resp = Response::error(msg.requestId(), Protocol::OpCode::Login,
                                       Protocol::StatusCode::Unauthorized,
                                       "账号已禁用");
        m_tcpServer->sendToClient(client, resp);
        return;
    }

    Message resp = Response::loginSuccess(msg.requestId(), user.id(),
                                          user.phone(), user.nickname(),
                                          user.walletBalance());
    m_tcpServer->sendToClient(client, resp);
    qDebug() << "登录成功 用户:" << user.nickname();
}

void ServerService::handleRegister(QTcpSocket *client, const Message &msg)
{
    QJsonObject payload = msg.payload();
    QString username = payload["username"].toString();
    QString phone = payload["phone"].toString();
    QString password = payload["password"].toString();
    QString confirmPassword = payload["confirmPassword"].toString();

    qDebug() << "注册请求 手机号:" << phone << " 用户名:" << username;

    if (phone.length() != 11) {
        Message resp = Response::error(msg.requestId(), Protocol::OpCode::Register,
                                       Protocol::StatusCode::BadRequest,
                                       "手机号必须为11位");
        m_tcpServer->sendToClient(client, resp);
        return;
    }

    if (password != confirmPassword) {
        Message resp = Response::error(msg.requestId(), Protocol::OpCode::Register,
                                       Protocol::StatusCode::BadRequest,
                                       "两次密码输入不一致");
        m_tcpServer->sendToClient(client, resp);
        return;
    }

    auto existingUser = AppUserDao::findByPhone(phone);
    if (existingUser.has_value()) {
        Message resp = Response::error(msg.requestId(), Protocol::OpCode::Register,
                                       Protocol::StatusCode::BadRequest,
                                       "该手机号已注册");
        m_tcpServer->sendToClient(client, resp);
        return;
    }

    QString hashedPassword = PasswordHasher::hash(password);
    int userId = AppUserDao::insert(phone, username, hashedPassword);
    if (userId < 0) {
        Message resp = Response::error(msg.requestId(), Protocol::OpCode::Register,
                                       Protocol::StatusCode::ServerError,
                                       "注册失败，数据库错误");
        m_tcpServer->sendToClient(client, resp);
        return;
    }

    Message resp = Response::registerSuccess(msg.requestId(), userId);
    m_tcpServer->sendToClient(client, resp);
    qDebug() << "注册成功 userId:" << userId;
}
