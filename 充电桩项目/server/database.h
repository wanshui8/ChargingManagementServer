#ifndef DATABASE_H
#define DATABASE_H

#include <QList>
#include <QPair>
#include <QString>
#include <QDateTime>
#include <QDate>
#include <QMap>
#include <QObject>
#include <QVariant>

#include "datatypes.h"

// ============================================================================
// 数据库管理类：封装 QSQLite 的建表、种子数据与全部 CRUD
// 注意：本类只在"业务线程"中实例化并被调用，保证 SQLite 连接的线程一致性
// ============================================================================
class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager();

    // 打开/创建数据库文件，建表，初始化种子数据；返回是否成功
    bool initialize(const QString &dbFilePath, QString &errorString);

    // ---------- 用户 ----------
    UserInfo   findOrCreateUser(const QString &phone);              // 免密登录/注册
    UserInfo   getUserById(int userId);
    void       setUserStatus(int userId, const QString &status);    // 冻结/解冻
    void       updateBalance(int userId, double newBalance);
    void       updateProfile(int userId, const QString &nickname, const QString &avatar);
    QList<UserInfo> searchUsers(const QString &keyword);

    // ---------- 充电站 ----------
    QList<StationInfo> getAllStations();
    StationInfo getStationById(int stationId);
    int        addStation(const StationInfo &station);              // 返回新 ID

    // ---------- 充电桩 ----------
    QList<PileInfo> getPilesByStation(int stationId);
    QList<PileInfo> getAllPiles();
    PileInfo getPileById(int pileId);
    int     addPile(int stationId, const QString &code, const QString &type, double powerKw);
    void     setPileStatus(int pileId, const QString &status);
    void     incrementPileStats(int pileId, int chargeMinutes);     // 充电次数+1、时长累加

    // ---------- 订单 ----------
    int        createOrder(int userId, int pileId, int stationId); // 创建充电中订单，返回订单 ID
    OrderInfo  getOrderById(int orderId);
    void       finishOrder(int orderId, double kwh, double fee);    // 结算订单
    OrderInfo  getActiveOrder(int userId);                          // 查询"充电中"订单（无则 id=0）
    QList<OrderInfo> getOrdersByUser(int userId);
    QList<OrderInfo> getAllOrders();                             // 全部订单（管理端查看）

    // ---------- 统计 ----------
    double getRevenueOfDay(const QDate &date);                      // 某日营收
    double getTotalRevenue();
    QList<QPair<QString, double>> getRevenueTrend(int days);        // 近 N 日营收趋势（日期, 金额）
    QMap<QString, int> getPileStatusSummary();                      // 各状态电桩数量(key: idle/charging/fault/offline)

    // ---------- 管理员 ----------
    bool verifyAdmin(const QString &username, const QString &password);

private:
    void createTables();
    void seedData();


    QString m_dbFilePath;
    QString m_connName;
};

#endif // DATABASE_H