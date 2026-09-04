#include "tcpclient.h"

#include <QTcpSocket>
#include <QTimer>
#include <QJsonDocument>
#include <QHostAddress>

TcpClient::TcpClient(QObject *parent)
    : QObject(parent)
{
    m_socket = new QTcpSocket(this);

    connect(m_socket, &QTcpSocket::connected, this, &TcpClient::onSocketConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpClient::onSocketDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &TcpClient::onSocketError);

    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(30000);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &TcpClient::onHeartbeat);
}

void TcpClient::connectToServer(const QString &host, quint16 port)
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->abort();
    }
    m_socket->connectToHost(host, port);
}

void TcpClient::disconnectFromServer()
{
    m_heartbeatTimer->stop();
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }
}

bool TcpClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void TcpClient::send(quint16 msgId, const QJsonObject &payload)
{
    if (!isConnected()) {
        return;
    }
    m_socket->write(Protocol::buildPacket(msgId, payload));
}

void TcpClient::onSocketConnected()
{
    m_heartbeatTimer->start();
    emit connected();
}

void TcpClient::onSocketDisconnected()
{
    m_heartbeatTimer->stop();
    m_parser.reset();
    emit disconnected();
}

void TcpClient::onReadyRead()
{
    m_parser.append(m_socket->readAll());

    quint16 msgId = 0;
    QJsonObject payload;
    while (m_parser.nextPacket(msgId, payload)) {
        emit messageReceived(msgId, payload);
    }
}

void TcpClient::onSocketError()
{
    emit errorOccurred(m_socket->errorString());
}

void TcpClient::onHeartbeat()
{
    send(Protocol::MsgId::REQ_HEARTBEAT, QJsonObject{});
}