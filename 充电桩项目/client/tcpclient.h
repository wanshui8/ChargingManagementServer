#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QJsonObject>

#include "protocol.h"
#include "datatypes.h"

class QTcpSocket;
class QTimer;

// ============================================================================
// 用户端网络客户端：与服务器端建立 TCP 长连接，收发协议消息，维护心跳
// 说明：QTcpSocket 为异步非阻塞，运行于 UI 主线程即可，不会阻塞界面。
// ============================================================================
class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);

    void connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();
    bool isConnected() const;

    // 发送一条消息
    void send(quint16 msgId, const QJsonObject &payload);

    // 当前登录用户
    UserInfo currentUser() const { return m_currentUser; }
    void setCurrentUser(const UserInfo &user) { m_currentUser = user; }

signals:
    void connected();
    void disconnected();
    void messageReceived(quint16 msgId, const QJsonObject &payload);
    void errorOccurred(const QString &error);

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onReadyRead();
    void onSocketError();
    void onHeartbeat();

private:
    QTcpSocket *m_socket = nullptr;
    QTimer     *m_heartbeatTimer = nullptr;

    Protocol::PacketParser m_parser;
    UserInfo m_currentUser;
};

#endif // TCPCLIENT_H