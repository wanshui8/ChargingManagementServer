#ifndef SERVERSERVICE_H
#define SERVERSERVICE_H

#include <QObject>
#include <QList>
#include <QPair>
#include <QMap>
#include <QString>
#include <QJsonObject>

#include "database.h"

// 前向声明，具体结构定义在 .cpp（使用指针避免头文件暴露实现细节）
struct ClientContext;
struct ChargeSession;

// 1. 先定义两个类型别名，避开宏对逗号的误判
using QStringDoublePair = QPair<QString, double>;
using QStringDoublePairList = QList<QStringDoublePair>;

// 2. 使用别名进行元类型注册
Q_DECLARE_METATYPE(QStringDoublePair)
Q_DECLARE_METATYPE(QStringDoublePairList)

class QTcpServer;
class QTcpSocket;
class QTimer;

// ============================================================================
// 服务器核心服务：网络监听 + 连接管理 + 充电业务 + 数据库访问
// 说明：本对象被 moveToThread 到独立"业务线程"，与 UI 主线程通过信号槽通信，
//       从而满足"多线程结构"要求，同时保证 QSQLite 只在业务线程被访问。
// ============================================================================
class ServerService : public QObject
{
    Q_OBJECT
public:
    explicit ServerService(QObject *parent = nullptr);
    ~ServerService() override;

    // 绑定数据库（在业务线程内调用）
    Q_INVOKABLE void initializeDatabase(const QString &dbFilePath);

public slots:
    // ---------- 网络控制（UI 调用） ----------
    void startServer(quint16 port);
    void stopServer();

    // ---------- 管理界面数据（UI 调用，结果经信号回传） ----------
    void asyncAdminLogin(const QString &username, const QString &password);
    void asyncLoadStations();
    void asyncLoadPiles();
    void asyncLoadUsers();
    void asyncLoadRevenue();
    void asyncLoadPileStatus();
    void asyncLoadOrders();
    void asyncAddStation(const QString &name, const QString &address,
                         double longitude, double latitude, double price, int pileCount);
    void asyncSetUserStatus(int userId, const QString &status);
    void asyncRestartPile(int pileId);

signals:
    // ---------- 网络状态 ----------
    void listenStateChanged(bool listening, quint16 port, const QString &address);

    // ---------- 管理界面数据回传 ----------
    void adminLoginResult(bool ok);
    void stationsReady(const QList<StationInfo> &stations);
    void pilesReady(const QList<PileInfo> &piles);
    void usersReady(const QList<UserInfo> &users);
    void revenueReady(const QList<QPair<QString, double>> &trend,
                      double today, double month, double total);
    void pileStatusReady(const QMap<QString, int> &summary);
    void ordersReady(const QList<OrderInfo> &orders);

    // ---------- 数据变更/日志 ----------
    void dataChanged();
    void logMessage(const QString &message);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();
    void onHeartbeatTimeout();
    void onChargeTick();

private:
    // 业务分发
    void dispatch(QTcpSocket *socket, quint16 msgId, const QJsonObject &payload);
    void sendTo(QTcpSocket *socket, quint16 msgId, const QJsonObject &payload);

    // 各消息处理器
    void handleLogin(ClientContext *ctx, const QJsonObject &payload);
    void handleStationList(ClientContext *ctx, const QJsonObject &payload);
    void handlePileList(ClientContext *ctx, const QJsonObject &payload);
    void handleCheckPending(ClientContext *ctx, const QJsonObject &payload);
    void handleStartCharge(ClientContext *ctx, const QJsonObject &payload);
    void handleStopCharge(ClientContext *ctx, const QJsonObject &payload);
    void handleRecharge(ClientContext *ctx, const QJsonObject &payload);
    void handleUpdateProfile(ClientContext *ctx, const QJsonObject &payload);
    void handleMyOrders(ClientContext *ctx, const QJsonObject &payload);
    void handleHeartbeat(ClientContext *ctx, const QJsonObject &payload);

    ClientContext* findContext(QTcpSocket *socket);
    // 结算一个充电会话（更新订单、恢复电桩、累计时长、扣余额），返回结算后订单
    OrderInfo settleChargeSession(ChargeSession *session, int elapsedSec);

    DatabaseManager m_db;
    QTcpServer      *m_server = nullptr;
    QTimer          *m_heartbeatTimer = nullptr;
    QTimer          *m_chargeTimer = nullptr;

    QMap<QTcpSocket*, ClientContext*>  m_clients;   // 连接池（socket -> context）
    QMap<int, ChargeSession*>          m_sessions;  // 充电会话（orderId -> session）

    quint16 m_port = 0;
};

#endif // SERVERSERVICE_H
