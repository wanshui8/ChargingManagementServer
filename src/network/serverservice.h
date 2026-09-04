#ifndef SERVERSERVICE_H
#define SERVERSERVICE_H

#include <QObject>
#include <QTcpSocket>
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

private slots:
    void onMessageReceived(QTcpSocket *client, const Message &msg);

private:
    void handleLogin(QTcpSocket *client, const Message &msg);

    TcpServer *m_tcpServer;
};

#endif // SERVERSERVICE_H
