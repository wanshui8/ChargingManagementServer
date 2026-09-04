#include "serverservice.h"
#include "protocol.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDateTime>
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>
#include <QDebug>
#include <QThread>

using namespace Protocol;



// ============================================================================
// 连接上下文：一个 TCP 连接对应一个客户端，维护拆包缓冲与登录身份
// ============================================================================
struct ClientContext {
    QTcpSocket *socket = nullptr;
    Protocol::PacketParser parser;
    int     userId = 0;      // 已登录用户 ID（0 表示未登录）
    QString phone;
    QDateTime lastActive;
};

// ============================================================================
// 充电会话：模拟一条进行中的充电，由定时器周期性推进并推送数据
// ============================================================================
struct ChargeSession {
    int orderId = 0;
    int userId = 0;
    int pileId = 0;
    int stationId = 0;
    QTcpSocket *socket = nullptr;
    double voltage = 0.0;
    double current = 0.0;
    double power = 0.0;     // kW
    double soc = 30.0;      // 初始电量 %
    double kwh = 0.0;       // 已充电量
    bool   fast = false;
    double pricePerKwh = 0.0;
    QDateTime startTime;
};

namespace {

// 球面距离（Haversine 简化），单位公里
double distanceKm(double lng1, double lat1, double lng2, double lat2)
{
    const double R = 6371.0;
    double dLat = (lat2 - lat1) * 3.141592653589793 / 180.0;
    double dLng = (lng2 - lng1) * 3.141592653589793 / 180.0;
    double a = qSin(dLat / 2) * qSin(dLat / 2) +
               qCos(lat1 * 3.141592653589793 / 180.0) * qCos(lat2 * 3.141592653589793 / 180.0) *
                   qSin(dLng / 2) * qSin(dLng / 2);
    double c = 2 * qAtan2(qSqrt(a), qSqrt(1 - a));
    return R * c;
}

// 订单 -> JSON
QJsonObject orderToJson(const OrderInfo &o)
{
    return QJsonObject{
        { "id", o.id },
        { "user_id", o.userId },
        { "pile_id", o.pileId },
        { "station_id", o.stationId },
        { "status", o.status },
        { "start_time", o.startTime },
        { "end_time", o.endTime },
        { "kwh", o.kwh },
        { "fee", o.fee }
    };
}

// 用户 -> JSON
QJsonObject userToJson(const UserInfo &u)
{
    return QJsonObject{
        { "id", u.id },
        { "phone", u.phone },
        { "nickname", u.nickname },
        { "avatar", u.avatar },
        { "balance", u.balance },
        { "status", u.status },
        { "create_time", u.createTime }
    };
}

// 电桩 -> JSON
QJsonObject pileToJson(const PileInfo &p)
{
    return QJsonObject{
        { "id", p.id },
        { "station_id", p.stationId },
        { "code", p.code },
        { "type", p.type },
        { "power_kw", p.powerKw },
        { "status", p.status },
        { "charge_count", p.chargeCount },
        { "charge_minutes", p.chargeMinutes }
    };
}

} // namespace

// ============================================================================
// 构造 / 析构
// ============================================================================
ServerService::ServerService(QObject *parent)
    : QObject(parent)
{
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &ServerService::onNewConnection);

    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(30000);   // 每 30 秒做一次超时检测
    connect(m_heartbeatTimer, &QTimer::timeout, this, &ServerService::onHeartbeatTimeout);

    m_chargeTimer = new QTimer(this);
    m_chargeTimer->setInterval(1000);       // 每秒推进一次充电并推送数据
    connect(m_chargeTimer, &QTimer::timeout, this, &ServerService::onChargeTick);
}

ServerService::~ServerService()
{
    stopServer();
}

void ServerService::initializeDatabase(const QString &dbFilePath)
{
    QString error;
    if (!m_db.initialize(dbFilePath, error)) {
        emit logMessage("数据库初始化失败: " + error);
    } else {
        emit logMessage("数据库初始化成功: " + dbFilePath);
    }
}

// ============================================================================
// 网络控制
// ============================================================================
void ServerService::startServer(quint16 port)
{
    if (m_server->isListening()) {
        return;
    }
    if (m_server->listen(QHostAddress::Any, port)) {
        m_port = port;
        m_heartbeatTimer->start();
        m_chargeTimer->start();
        emit listenStateChanged(true, port, m_server->serverAddress().toString());
        emit logMessage(QString("服务器已启动，监听端口 %1").arg(port));
    } else {
        emit listenStateChanged(false, port, QString());
        emit logMessage("监听失败: " + m_server->errorString());
    }
}

void ServerService::stopServer()
{
    m_chargeTimer->stop();
    m_heartbeatTimer->stop();

    qDeleteAll(m_sessions);
    m_sessions.clear();

    for (ClientContext *ctx : m_clients) {
        if (ctx->socket) {
            ctx->socket->disconnectFromHost();
        }
    }
    qDeleteAll(m_clients);
    m_clients.clear();

    if (m_server->isListening()) {
        m_server->close();
    }
    emit listenStateChanged(false, m_port, QString());
    emit logMessage("服务器已停止");
}

// ============================================================================
// 管理界面数据（槽函数，结果经信号回传）
// ============================================================================
void ServerService::asyncAdminLogin(const QString &username, const QString &password)
{
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, "asyncAdminLogin", Qt::QueuedConnection,
                                  Q_ARG(QString, username), Q_ARG(QString, password));
        return;
    }
    emit adminLoginResult(m_db.verifyAdmin(username, password));
}

void ServerService::asyncLoadStations()
{
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, "asyncLoadStations", Qt::QueuedConnection);
        return;
    }
    emit stationsReady(m_db.getAllStations());
}

void ServerService::asyncLoadPiles()
{
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, "asyncLoadPiles", Qt::QueuedConnection);
        return;
    }
    emit pilesReady(m_db.getAllPiles());
}

void ServerService::asyncLoadUsers()
{
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, "asyncLoadUsers", Qt::QueuedConnection);
        return;
    }
    emit usersReady(m_db.searchUsers(QString()));
}

void ServerService::asyncLoadRevenue()
{
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, "asyncLoadRevenue", Qt::QueuedConnection);
        return;
    }
    QDate today = QDate::currentDate();
    int year = today.year();
    int month = today.month();

    double todayRevenue = m_db.getRevenueOfDay(today);
    double monthRevenue = 0.0;
    for (int d = 1; d <= today.day(); ++d) {
        monthRevenue += m_db.getRevenueOfDay(QDate(year, month, d));
    }
    double totalRevenue = m_db.getTotalRevenue();
    QList<QPair<QString, double>> trend = m_db.getRevenueTrend(30);

    emit revenueReady(trend, todayRevenue, monthRevenue, totalRevenue);
}

void ServerService::asyncLoadPileStatus()
{
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, "asyncLoadPileStatus", Qt::QueuedConnection);
        return;
    }
    emit pileStatusReady(m_db.getPileStatusSummary());
}

void ServerService::asyncLoadOrders()
{
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, "asyncLoadOrders", Qt::QueuedConnection);
        return;
    }
    emit ordersReady(m_db.getAllOrders());
}

void ServerService::asyncAddStation(const QString &name, const QString &address,
                                    double longitude, double latitude, double price, int pileCount)
{
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, "asyncAddStation", Qt::QueuedConnection,
                                  Q_ARG(QString, name), Q_ARG(QString, address),
                                  Q_ARG(double, longitude), Q_ARG(double, latitude),
                                  Q_ARG(double, price), Q_ARG(int, pileCount));
        return;
    }
    StationInfo s;
    s.name = name;
    s.address = address;
    s.longitude = longitude;
    s.latitude = latitude;
    s.pricePerKwh = price;
    int stationId = m_db.addStation(s);

    // 为新电站批量创建空闲电桩（1 快充 + 其余慢充）
    for (int i = 0; i < pileCount; ++i) {
        bool fast = (i == 0);
        m_db.addPile(stationId,
                     QString("CP-%1").arg(stationId * 100 + i + 1),
                     fast ? "fast" : "slow",
                     fast ? 120.0 : 7.0);
    }
    emit logMessage(QString("新增充电站: %1（%2 个电桩）").arg(name).arg(pileCount));
    emit dataChanged();
    asyncLoadStations();
}

void ServerService::asyncSetUserStatus(int userId, const QString &status)
{
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, "asyncSetUserStatus", Qt::QueuedConnection,
                                  Q_ARG(int, userId), Q_ARG(QString, status));
        return;
    }
    m_db.setUserStatus(userId, status);
    emit logMessage(QString("用户 %1 状态更新为 %2").arg(userId).arg(status));
    emit dataChanged();
    asyncLoadUsers();
}

void ServerService::asyncRestartPile(int pileId)
{
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, "asyncRestartPile", Qt::QueuedConnection,
                                  Q_ARG(int, pileId));
        return;
    }
    PileInfo p = m_db.getPileById(pileId);
    if (p.id == 0) {
        emit logMessage("电桩不存在");
        return;
    }
    // 模拟远程重启：故障桩恢复为空闲
    m_db.setPileStatus(pileId, "idle");
    emit logMessage(QString("电桩 %1 已远程重启").arg(p.code));
    emit dataChanged();
    asyncLoadPiles();
}

// ============================================================================
// 网络事件处理
// ============================================================================
void ServerService::onNewConnection()
{
    while (QTcpSocket *socket = m_server->nextPendingConnection()) {
        ClientContext *ctx = new ClientContext;
        ctx->socket = socket;
        ctx->lastActive = QDateTime::currentDateTime();

        connect(socket, &QTcpSocket::readyRead, this, &ServerService::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &ServerService::onDisconnected);

        m_clients.insert(socket, ctx);
        emit logMessage(QString("新连接: %1:%2")
                            .arg(socket->peerAddress().toString())
                            .arg(socket->peerPort()));
        emit dataChanged();
    }
}

void ServerService::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) {
        return;
    }
    ClientContext *ctx = findContext(socket);
    if (!ctx) {
        return;
    }

    ctx->parser.append(socket->readAll());
    ctx->lastActive = QDateTime::currentDateTime();

    quint16 msgId = 0;
    QJsonObject payload;
    while (ctx->parser.nextPacket(msgId, payload)) {
        dispatch(socket, msgId, payload);
    }
}

void ServerService::onDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) {
        return;
    }

    // 若该连接存在活跃充电会话，则自动结算，避免电桩一直占用
    for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
        if (it.value()->socket == socket) {
            ChargeSession *session = it.value();
            int elapsedSec = static_cast<int>(session->startTime.secsTo(QDateTime::currentDateTime()));
            settleChargeSession(session, elapsedSec);
            emit logMessage(QString("断线自动结算订单 %1").arg(session->orderId));
            delete session;
            m_sessions.erase(it);
            break;
        }
    }

    ClientContext *ctx = findContext(socket);
    if (ctx) {
        m_clients.remove(socket);
        delete ctx;
    }
    socket->deleteLater();

    emit logMessage("连接断开");
    emit dataChanged();
}

void ServerService::onHeartbeatTimeout()
{
    QDateTime now = QDateTime::currentDateTime();
    QList<QTcpSocket *> toClose;
    for (ClientContext *ctx : m_clients) {
        if (ctx->lastActive.msecsTo(now) > 90000) {  // 超过 90 秒无数据视为离线
            toClose.append(ctx->socket);
        }
    }
    for (QTcpSocket *s : toClose) {
        s->disconnectFromHost();
        emit logMessage("连接超时，已断开");
    }
}

// ============================================================================
// 充电推进（每秒一次）
// ============================================================================
void ServerService::onChargeTick()
{
    QDateTime now = QDateTime::currentDateTime();
    QRandomGenerator *rng = QRandomGenerator::global();

    for (auto it = m_sessions.begin(); it != m_sessions.end();) {
        ChargeSession *s = it.value();

        int elapsedSec = static_cast<int>(s->startTime.secsTo(now));
        if (elapsedSec < 0) elapsedSec = 0;

        // 模拟电压：快充直流 500V，慢充交流 220V，带随机波动
        double baseVoltage = s->fast ? 500.0 : 220.0;
        s->voltage = baseVoltage + (rng->bounded(20) - 10);

        // 模拟电流：快充 80~120A，慢充约 16A
        double baseCurrent = s->fast ? 100.0 : 16.0;
        s->current = baseCurrent + (rng->bounded(8) - 4);

        // 功率(kW) = 电压(V) * 电流(A) / 1000
        s->power = s->voltage * s->current / 1000.0;

        // SOC 增长：快充 0.4%/秒，慢充 0.1%/秒
        s->soc += (s->fast ? 0.4 : 0.1);
        if (s->soc > 100.0) s->soc = 100.0;

        // 电量累计：功率(kW) × 时间(秒) / 3600 = kWh
        s->kwh += s->power / 3600.0;

        QJsonObject data;
        data["order_id"] = s->orderId;
        data["voltage"] = s->voltage;
        data["current"] = s->current;
        data["power"] = s->power;
        data["soc"] = s->soc;
        data["kwh"] = s->kwh;
        sendTo(s->socket, MsgId::RES_CHARGE_DATA, data);

        // SOC 充满自动停止并结算
        if (s->soc >= 100.0) {
            OrderInfo settled = settleChargeSession(s, elapsedSec);
            double newBalance = m_db.getUserById(s->userId).balance;

            QJsonObject res;
            res["code"] = ErrorCode::OK;
            res["order"] = orderToJson(settled);
            res["balance"] = newBalance;
            res["auto"] = true;
            sendTo(s->socket, MsgId::RES_STOP_CHARGE, res);

            delete s;
            it = m_sessions.erase(it);
            emit dataChanged();
            continue;
        }

        ++it;
    }
}

// ============================================================================
// 消息分发
// ============================================================================
void ServerService::dispatch(QTcpSocket *socket, quint16 msgId, const QJsonObject &payload)
{
    ClientContext *ctx = findContext(socket);
    if (!ctx) {
        return;
    }
    switch (msgId) {
    case MsgId::REQ_LOGIN:          handleLogin(ctx, payload);          break;
    case MsgId::REQ_STATION_LIST:   handleStationList(ctx, payload);    break;
    case MsgId::REQ_PILE_LIST:      handlePileList(ctx, payload);       break;
    case MsgId::REQ_CHECK_PENDING:  handleCheckPending(ctx, payload);   break;
    case MsgId::REQ_START_CHARGE:   handleStartCharge(ctx, payload);    break;
    case MsgId::REQ_STOP_CHARGE:    handleStopCharge(ctx, payload);     break;
    case MsgId::REQ_RECHARGE:       handleRecharge(ctx, payload);       break;
    case MsgId::REQ_UPDATE_PROFILE: handleUpdateProfile(ctx, payload);  break;
    case MsgId::REQ_MY_ORDERS:      handleMyOrders(ctx, payload);       break;
    case MsgId::REQ_HEARTBEAT:      handleHeartbeat(ctx, payload);      break;
    default:
        emit logMessage(QString("未知消息类型: %1").arg(msgId));
        break;
    }
}

void ServerService::sendTo(QTcpSocket *socket, quint16 msgId, const QJsonObject &payload)
{
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
        return;
    }
    QByteArray packet = Protocol::buildPacket(msgId, payload);
    socket->write(packet);
}

ClientContext *ServerService::findContext(QTcpSocket *socket)
{
    auto it = m_clients.find(socket);
    return (it != m_clients.end()) ? it.value() : nullptr;
}

OrderInfo ServerService::settleChargeSession(ChargeSession *session, int elapsedSec)
{
    double fee = session->kwh * session->pricePerKwh;
    int chargeMinutes = qMax(0, elapsedSec) / 60;

    m_db.finishOrder(session->orderId, session->kwh, fee);
    m_db.setPileStatus(session->pileId, PileStatus::IDLE);
    m_db.incrementPileStats(session->pileId, chargeMinutes);

    UserInfo user = m_db.getUserById(session->userId);
    m_db.updateBalance(session->userId, user.balance - fee);

    return m_db.getOrderById(session->orderId);
}

// ============================================================================
// 各消息处理器
// ============================================================================
void ServerService::handleLogin(ClientContext *ctx, const QJsonObject &payload)
{
    QString phone = payload.value("phone").toString();
    if (phone.isEmpty()) {
        sendTo(ctx->socket, MsgId::RES_LOGIN, QJsonObject{ { "code", ErrorCode::INVALID_MESSAGE } });
        return;
    }

    UserInfo user = m_db.findOrCreateUser(phone);
    if (user.status == UserStatus::FROZEN) {
        sendTo(ctx->socket, MsgId::RES_LOGIN, QJsonObject{ { "code", ErrorCode::USER_FROZEN } });
        return;
    }

    ctx->userId = user.id;
    ctx->phone = phone;

    QJsonObject res;
    res["code"] = ErrorCode::OK;
    res["user"] = userToJson(user);
    sendTo(ctx->socket, MsgId::RES_LOGIN, res);
    emit logMessage(QString("用户登录: %1").arg(phone));
    emit dataChanged();
}

void ServerService::handleStationList(ClientContext *ctx, const QJsonObject &payload)
{
    double userLng = payload.value("longitude").toDouble();
    double userLat = payload.value("latitude").toDouble();

    QList<StationInfo> stations = m_db.getAllStations();

    // 按距离由近及远排序
    std::sort(stations.begin(), stations.end(), [&](const StationInfo &a, const StationInfo &b) {
        return distanceKm(userLng, userLat, a.longitude, a.latitude) <
               distanceKm(userLng, userLat, b.longitude, b.latitude);
    });

    QJsonArray arr;
    for (const StationInfo &s : stations) {
        QJsonObject o;
        o["id"] = s.id;
        o["name"] = s.name;
        o["address"] = s.address;
        o["longitude"] = s.longitude;
        o["latitude"] = s.latitude;
        o["price_per_kwh"] = s.pricePerKwh;
        o["total_piles"] = s.totalPiles;
        o["idle_piles"] = s.idlePiles;
        o["distance"] = distanceKm(userLng, userLat, s.longitude, s.latitude);
        arr.append(o);
    }

    QJsonObject res;
    res["code"] = ErrorCode::OK;
    res["stations"] = arr;
    sendTo(ctx->socket, MsgId::RES_STATION_LIST, res);
}

void ServerService::handlePileList(ClientContext *ctx, const QJsonObject &payload)
{
    int stationId = payload.value("station_id").toInt();
    QList<PileInfo> piles = m_db.getPilesByStation(stationId);

    QJsonArray arr;
    for (const PileInfo &p : piles) {
        arr.append(pileToJson(p));
    }

    QJsonObject res;
    res["code"] = ErrorCode::OK;
    res["piles"] = arr;
    sendTo(ctx->socket, MsgId::RES_PILE_LIST, res);
}

void ServerService::handleCheckPending(ClientContext *ctx, const QJsonObject &payload)
{
    int userId = payload.value("user_id").toInt();
    OrderInfo active = m_db.getActiveOrder(userId);

    QJsonObject res;
    res["code"] = ErrorCode::OK;
    if (active.id != 0) {
        res["has_pending"] = true;
        res["order"] = orderToJson(active);
    } else {
        res["has_pending"] = false;
    }
    sendTo(ctx->socket, MsgId::RES_CHECK_PENDING, res);
}

void ServerService::handleStartCharge(ClientContext *ctx, const QJsonObject &payload)
{
    int userId = payload.value("user_id").toInt();
    int pileId = payload.value("pile_id").toInt();

    QJsonObject res;

    // 检查未完成订单
    if (m_db.getActiveOrder(userId).id != 0) {
        res["code"] = ErrorCode::HAS_PENDING_ORDER;
        sendTo(ctx->socket, MsgId::RES_START_CHARGE, res);
        return;
    }

    PileInfo pile = m_db.getPileById(pileId);
    if (pile.id == 0) {
        res["code"] = ErrorCode::PILE_NOT_FOUND;
        sendTo(ctx->socket, MsgId::RES_START_CHARGE, res);
        return;
    }
    if (pile.status != PileStatus::IDLE) {
        res["code"] = ErrorCode::PILE_NOT_IDLE;
        sendTo(ctx->socket, MsgId::RES_START_CHARGE, res);
        return;
    }

    StationInfo station = m_db.getStationById(pile.stationId);

    // 创建订单 + 会话
    int orderId = m_db.createOrder(userId, pileId, pile.stationId);
    m_db.setPileStatus(pileId, PileStatus::CHARGING);

    ChargeSession *session = new ChargeSession;
    session->orderId = orderId;
    session->userId = userId;
    session->pileId = pileId;
    session->stationId = pile.stationId;
    session->socket = ctx->socket;
    session->fast = (pile.type == "fast");
    session->pricePerKwh = station.pricePerKwh;
    session->startTime = QDateTime::currentDateTime();
    m_sessions.insert(orderId, session);

    res["code"] = ErrorCode::OK;
    res["order"] = orderToJson(m_db.getOrderById(orderId));
    res["price_per_kwh"] = station.pricePerKwh;
    sendTo(ctx->socket, MsgId::RES_START_CHARGE, res);

    emit logMessage(QString("启动充电: 订单 %1, 电桩 %2").arg(orderId).arg(pile.code));
    emit dataChanged();
}

void ServerService::handleStopCharge(ClientContext *ctx, const QJsonObject &payload)
{
    int orderId = payload.value("order_id").toInt();
    QJsonObject res;

    auto it = m_sessions.find(orderId);
    if (it == m_sessions.end()) {
        res["code"] = ErrorCode::ORDER_NOT_FOUND;
        sendTo(ctx->socket, MsgId::RES_STOP_CHARGE, res);
        return;
    }

    ChargeSession *s = it.value();
    int elapsedSec = static_cast<int>(s->startTime.secsTo(QDateTime::currentDateTime()));
    OrderInfo settled = settleChargeSession(s, elapsedSec);
    double newBalance = m_db.getUserById(s->userId).balance;

    res["code"] = ErrorCode::OK;
    res["order"] = orderToJson(settled);
    res["balance"] = newBalance;
    res["auto"] = false;
    sendTo(ctx->socket, MsgId::RES_STOP_CHARGE, res);

    delete s;
    m_sessions.erase(it);

    emit logMessage(QString("停止充电并结算: 订单 %1, 费用 %2 元").arg(orderId).arg(settled.fee));
    emit dataChanged();
}

void ServerService::handleRecharge(ClientContext *ctx, const QJsonObject &payload)
{
    int userId = payload.value("user_id").toInt();
    double amount = payload.value("amount").toDouble();

    UserInfo user = m_db.getUserById(userId);
    double newBalance = user.balance + amount;
    m_db.updateBalance(userId, newBalance);

    QJsonObject res;
    res["code"] = ErrorCode::OK;
    res["balance"] = newBalance;
    sendTo(ctx->socket, MsgId::RES_RECHARGE, res);
    emit logMessage(QString("用户 %1 充值 %2 元").arg(userId).arg(amount));
}

void ServerService::handleUpdateProfile(ClientContext *ctx, const QJsonObject &payload)
{
    int userId = payload.value("user_id").toInt();
    QString nickname = payload.value("nickname").toString();
    QString avatar = payload.value("avatar").toString();

    m_db.updateProfile(userId, nickname, avatar);

    QJsonObject res;
    res["code"] = ErrorCode::OK;
    res["user"] = userToJson(m_db.getUserById(userId));
    sendTo(ctx->socket, MsgId::RES_UPDATE_PROFILE, res);
}

void ServerService::handleMyOrders(ClientContext *ctx, const QJsonObject &payload)
{
    int userId = payload.value("user_id").toInt();
    QList<OrderInfo> orders = m_db.getOrdersByUser(userId);

    QJsonArray arr;
    for (const OrderInfo &o : orders) {
        arr.append(orderToJson(o));
    }

    QJsonObject res;
    res["code"] = ErrorCode::OK;
    res["orders"] = arr;
    sendTo(ctx->socket, MsgId::RES_MY_ORDERS, res);
}

void ServerService::handleHeartbeat(ClientContext *ctx, const QJsonObject &payload)
{
    Q_UNUSED(payload);
    QJsonObject res;
    res["code"] = ErrorCode::OK;
    res["time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    sendTo(ctx->socket, MsgId::RES_HEARTBEAT, res);
}
