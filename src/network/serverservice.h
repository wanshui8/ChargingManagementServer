#ifndef SERVERSERVICE_H
#define SERVERSERVICE_H

#include <QObject>
#include <QTcpSocket>
#include <QHash>
#include "network/TcpServer.h"
#include "protocol/Message.h"

class ServerService : public QObject
{
    Q_OBJECT
public:
    explicit ServerService(QObject *parent = nullptr);
    ~ServerService();

    bool start(quint16 port = 9500);
    void stop();

    int getUserId(QTcpSocket *client) const;

private slots:
    void onMessageReceived(QTcpSocket *client, const Message &msg);
    void onClientDisconnected(QTcpSocket *client);

private:
    void handleLogin(QTcpSocket *client, const Message &msg);
    void handleRegister(QTcpSocket *client, const Message &msg);

    TcpServer *m_tcpServer;
    QHash<QTcpSocket*, int> m_socketToUserId;
};

#endif // SERVERSERVICE_H
