#include "database.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QCryptographicHash>
#include <QUuid>
#include <QDebug>

// ============================================================================
// 工具函数
// ============================================================================
namespace {

// 对密码做 SHA-256 哈希，避免明文存储（数据安全要求）
QString hashPassword(const QString &plain)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(plain.toUtf8(), QCryptographicHash::Sha256).toHex());
}

QString nowString()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}

} // namespace

// ============================================================================
// 构造 / 析构
// ============================================================================
DatabaseManager::DatabaseManager()
    : m_connName("server_db_" + QUuid::createUuid().toString(QUuid::WithoutBraces))
{
}

DatabaseManager::~DatabaseManager()
{
    {
        QSqlDatabase db = QSqlDatabase::database(m_connName, false);
        if (db.isOpen()) {
            db.close();
        }
    }
    QSqlDatabase::removeDatabase(m_connName);
}

bool DatabaseManager::initialize(const QString &dbFilePath, QString &errorString)
{
    m_dbFilePath = dbFilePath;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connName);
    db.setDatabaseName(m_dbFilePath);
    if (!db.open()) {
        errorString = db.lastError().text();
        return false;
    }

    createTables();
    seedData();
    return true;
}

void DatabaseManager::createTables()
{
    QSqlQuery q(QSqlDatabase::database(m_connName));

    q.exec("CREATE TABLE IF NOT EXISTS admin ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "username TEXT UNIQUE NOT NULL,"
           "password TEXT NOT NULL,"
           "role TEXT DEFAULT 'admin')");

    q.exec("CREATE TABLE IF NOT EXISTS user ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "phone TEXT UNIQUE NOT NULL,"
           "nickname TEXT,"
           "avatar TEXT,"
           "balance REAL DEFAULT 0,"
           "status TEXT DEFAULT 'normal',"
           "create_time TEXT)");

    q.exec("CREATE TABLE IF NOT EXISTS charging_station ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "name TEXT NOT NULL,"
           "address TEXT,"
           "longitude REAL DEFAULT 0,"
           "latitude REAL DEFAULT 0,"
           "price_per_kwh REAL DEFAULT 1.0)");

    q.exec("CREATE TABLE IF NOT EXISTS charging_pile ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "station_id INTEGER NOT NULL,"
           "code TEXT NOT NULL,"
           "type TEXT DEFAULT 'slow',"
           "power_kw REAL DEFAULT 0,"
           "status TEXT DEFAULT 'idle',"
           "charge_count INTEGER DEFAULT 0,"
           "charge_minutes INTEGER DEFAULT 0)");

    q.exec("CREATE TABLE IF NOT EXISTS charging_order ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "user_id INTEGER NOT NULL,"
           "pile_id INTEGER NOT NULL,"
           "station_id INTEGER NOT NULL,"
           "status TEXT DEFAULT 'charging',"
           "start_time TEXT,"
           "end_time TEXT,"
           "kwh REAL DEFAULT 0,"
           "fee REAL DEFAULT 0)");
}

void DatabaseManager::seedData()
{
    QSqlQuery q(QSqlDatabase::database(m_connName));

    // 默认管理员 admin / 123456（密码哈希存储）
    q.prepare("SELECT COUNT(*) FROM admin");
    q.exec();
    q.next();
    if (q.value(0).toInt() == 0) {
        QSqlQuery i(QSqlDatabase::database(m_connName));
        i.prepare("INSERT INTO admin(username, password) VALUES(?, ?)");
        i.addBindValue("admin");
        i.addBindValue(hashPassword("123456"));
        i.exec();
    }

    // 仅当无充电站时注入种子数据
    QSqlQuery c(QSqlDatabase::database(m_connName));
    c.exec("SELECT COUNT(*) FROM charging_station");
    c.next();
    if (c.value(0).toInt() > 0) {
        return;
    }

    // 种子充电站（经纬度示例，示意某城市区域）
    struct SeedStation { QString name; QString address; double lng; double lat; double price; int piles; };
    QList<SeedStation> stations = {
        { "东软科技园充电站", "滨江区东软科技园A区停车场",  120.21, 30.21, 1.20, 6 },
        { "市民中心充电站",   "滨江区市民中心地下车库B2",    120.20, 30.20, 1.10, 8 },
        { "高铁东站充电站",   "高铁东站南广场换乘中心",       120.23, 30.19, 1.30, 6 },
        { "万达广场充电站",   "万达广场地下停车场C区",       120.19, 30.22, 1.25, 5 },
        { "生态公园充电站",   "滨江生态公园西门停车场",       120.18, 30.18, 0.90, 4 },
        { "大学城充电站",     "大学城科技园南路",            120.22, 30.23, 1.00, 7 },
    };

    int pileIndex = 1;
    for (const SeedStation &s : stations) {
        QSqlQuery ins(QSqlDatabase::database(m_connName));
        ins.prepare("INSERT INTO charging_station(name, address, longitude, latitude, price_per_kwh)"
                    " VALUES(?, ?, ?, ?, ?)");
        ins.addBindValue(s.name);
        ins.addBindValue(s.address);
        ins.addBindValue(s.lng);
        ins.addBindValue(s.lat);
        ins.addBindValue(s.price);
        ins.exec();
        int stationId = ins.lastInsertId().toInt();

        for (int i = 0; i < s.piles; ++i) {
            bool fast = (i % 3 == 0);                       // 每 3 个桩取 1 个快充
            QString type = fast ? "fast" : "slow";
            double power = fast ? 120.0 : 7.0;

            // 构造不同初始状态，便于演示
            QString status = "idle";
            if (i % 7 == 3) status = "charging";
            if (i % 11 == 5) status = "fault";

            QSqlQuery ip(QSqlDatabase::database(m_connName));
            ip.prepare("INSERT INTO charging_pile(station_id, code, type, power_kw, status)"
                       " VALUES(?, ?, ?, ?, ?)");
            ip.addBindValue(stationId);
            ip.addBindValue(QString("CP-%1").arg(pileIndex, 4, 10, QChar('0')));
            ip.addBindValue(type);
            ip.addBindValue(power);
            ip.addBindValue(status);
            ip.exec();
            ++pileIndex;
        }
    }
}

// ============================================================================
// 用户
// ============================================================================
UserInfo DatabaseManager::findOrCreateUser(const QString &phone)
{
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("SELECT id, phone, nickname, avatar, balance, status, create_time FROM user WHERE phone = ?");
    q.addBindValue(phone);
    q.exec();
    if (q.next()) {
        UserInfo u;
        u.id = q.value(0).toInt();
        u.phone = q.value(1).toString();
        u.nickname = q.value(2).toString();
        u.avatar = q.value(3).toString();
        u.balance = q.value(4).toDouble();
        u.status = q.value(5).toString();
        u.createTime = q.value(6).toString();
        return u;
    }

    // 新用户自动注册：昵称默认"用户+手机号后4位"
    QString nick = "用户" + phone.right(4);
    QSqlQuery ins(QSqlDatabase::database(m_connName));
    ins.prepare("INSERT INTO user(phone, nickname, balance, status, create_time) VALUES(?, ?, 0, 'normal', ?)");
    ins.addBindValue(phone);
    ins.addBindValue(nick);
    ins.addBindValue(nowString());
    ins.exec();

    UserInfo u;
    u.id = ins.lastInsertId().toInt();
    u.phone = phone;
    u.nickname = nick;
    u.balance = 0.0;
    u.status = "normal";
    u.createTime = nowString();
    return u;
}

UserInfo DatabaseManager::getUserById(int userId)
{
    UserInfo u;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("SELECT id, phone, nickname, avatar, balance, status, create_time FROM user WHERE id = ?");
    q.addBindValue(userId);
    q.exec();
    if (q.next()) {
        u.id = q.value(0).toInt();
        u.phone = q.value(1).toString();
        u.nickname = q.value(2).toString();
        u.avatar = q.value(3).toString();
        u.balance = q.value(4).toDouble();
        u.status = q.value(5).toString();
        u.createTime = q.value(6).toString();
    }
    return u;
}

void DatabaseManager::setUserStatus(int userId, const QString &status)
{
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("UPDATE user SET status = ? WHERE id = ?");
    q.addBindValue(status);
    q.addBindValue(userId);
    q.exec();
}

void DatabaseManager::updateBalance(int userId, double newBalance)
{
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("UPDATE user SET balance = ? WHERE id = ?");
    q.addBindValue(newBalance);
    q.addBindValue(userId);
    q.exec();
}

void DatabaseManager::updateProfile(int userId, const QString &nickname, const QString &avatar)
{
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("UPDATE user SET nickname = ?, avatar = ? WHERE id = ?");
    q.addBindValue(nickname);
    q.addBindValue(avatar);
    q.addBindValue(userId);
    q.exec();
}

QList<UserInfo> DatabaseManager::searchUsers(const QString &keyword)
{
    QList<UserInfo> list;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    if (keyword.isEmpty()) {
        q.prepare("SELECT id, phone, nickname, avatar, balance, status, create_time FROM user");
    } else {
        q.prepare("SELECT id, phone, nickname, avatar, balance, status, create_time FROM user WHERE phone LIKE ? OR nickname LIKE ?");
        q.addBindValue("%" + keyword + "%");
        q.addBindValue("%" + keyword + "%");
    }
    q.exec();
    while (q.next()) {
        UserInfo u;
        u.id = q.value(0).toInt();
        u.phone = q.value(1).toString();
        u.nickname = q.value(2).toString();
        u.avatar = q.value(3).toString();
        u.balance = q.value(4).toDouble();
        u.status = q.value(5).toString();
        u.createTime = q.value(6).toString();
        list.append(u);
    }
    return list;
}

// ============================================================================
// 充电站
// ============================================================================
QList<StationInfo> DatabaseManager::getAllStations()
{
    QList<StationInfo> list;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.exec("SELECT id, name, address, longitude, latitude, price_per_kwh FROM charging_station");
    while (q.next()) {
        StationInfo s;
        s.id = q.value(0).toInt();
        s.name = q.value(1).toString();
        s.address = q.value(2).toString();
        s.longitude = q.value(3).toDouble();
        s.latitude = q.value(4).toDouble();
        s.pricePerKwh = q.value(5).toDouble();

        // 统计桩数量
        QSqlQuery p(QSqlDatabase::database(m_connName));
        p.prepare("SELECT status FROM charging_pile WHERE station_id = ?");
        p.addBindValue(s.id);
        p.exec();
        while (p.next()) {
            s.totalPiles++;
            QString st = p.value(0).toString();
            if (st == "idle") s.idlePiles++;
            if (st != "offline") s.onlinePiles++;
        }
        list.append(s);
    }
    return list;
}

StationInfo DatabaseManager::getStationById(int stationId)
{
    StationInfo s;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("SELECT id, name, address, longitude, latitude, price_per_kwh FROM charging_station WHERE id = ?");
    q.addBindValue(stationId);
    q.exec();
    if (q.next()) {
        s.id = q.value(0).toInt();
        s.name = q.value(1).toString();
        s.address = q.value(2).toString();
        s.longitude = q.value(3).toDouble();
        s.latitude = q.value(4).toDouble();
        s.pricePerKwh = q.value(5).toDouble();
    }
    return s;
}

int DatabaseManager::addStation(const StationInfo &station)
{
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("INSERT INTO charging_station(name, address, longitude, latitude, price_per_kwh)"
              " VALUES(?, ?, ?, ?, ?)");
    q.addBindValue(station.name);
    q.addBindValue(station.address);
    q.addBindValue(station.longitude);
    q.addBindValue(station.latitude);
    q.addBindValue(station.pricePerKwh);
    q.exec();
    return q.lastInsertId().toInt();
}

// ============================================================================
// 充电桩
// ============================================================================
QList<PileInfo> DatabaseManager::getPilesByStation(int stationId)
{
    QList<PileInfo> list;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("SELECT id, station_id, code, type, power_kw, status, charge_count, charge_minutes FROM charging_pile WHERE station_id = ? ORDER BY id");
    q.addBindValue(stationId);
    q.exec();
    while (q.next()) {
        PileInfo p;
        p.id = q.value(0).toInt();
        p.stationId = q.value(1).toInt();
        p.code = q.value(2).toString();
        p.type = q.value(3).toString();
        p.powerKw = q.value(4).toDouble();
        p.status = q.value(5).toString();
        p.chargeCount = q.value(6).toInt();
        p.chargeMinutes = q.value(7).toInt();
        list.append(p);
    }
    return list;
}

QList<PileInfo> DatabaseManager::getAllPiles()
{
    QList<PileInfo> list;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.exec("SELECT id, station_id, code, type, power_kw, status, charge_count, charge_minutes FROM charging_pile ORDER BY id");
    while (q.next()) {
        PileInfo p;
        p.id = q.value(0).toInt();
        p.stationId = q.value(1).toInt();
        p.code = q.value(2).toString();
        p.type = q.value(3).toString();
        p.powerKw = q.value(4).toDouble();
        p.status = q.value(5).toString();
        p.chargeCount = q.value(6).toInt();
        p.chargeMinutes = q.value(7).toInt();
        list.append(p);
    }
    return list;
}

PileInfo DatabaseManager::getPileById(int pileId)
{
    PileInfo p;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("SELECT id, station_id, code, type, power_kw, status, charge_count, charge_minutes FROM charging_pile WHERE id = ?");
    q.addBindValue(pileId);
    q.exec();
    if (q.next()) {
        p.id = q.value(0).toInt();
        p.stationId = q.value(1).toInt();
        p.code = q.value(2).toString();
        p.type = q.value(3).toString();
        p.powerKw = q.value(4).toDouble();
        p.status = q.value(5).toString();
        p.chargeCount = q.value(6).toInt();
        p.chargeMinutes = q.value(7).toInt();
    }
    return p;
}

int DatabaseManager::addPile(int stationId, const QString &code, const QString &type, double powerKw)
{
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("INSERT INTO charging_pile(station_id, code, type, power_kw, status)"
              " VALUES(?, ?, ?, ?, 'idle')");
    q.addBindValue(stationId);
    q.addBindValue(code);
    q.addBindValue(type);
    q.addBindValue(powerKw);
    q.exec();
    return q.lastInsertId().toInt();
}

void DatabaseManager::setPileStatus(int pileId, const QString &status)
{
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("UPDATE charging_pile SET status = ? WHERE id = ?");
    q.addBindValue(status);
    q.addBindValue(pileId);
    q.exec();
}

void DatabaseManager::incrementPileStats(int pileId, int chargeMinutes)
{
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("UPDATE charging_pile SET charge_count = charge_count + 1, charge_minutes = charge_minutes + ? WHERE id = ?");
    q.addBindValue(chargeMinutes);
    q.addBindValue(pileId);
    q.exec();
}

// ============================================================================
// 订单
// ============================================================================
int DatabaseManager::createOrder(int userId, int pileId, int stationId)
{
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("INSERT INTO charging_order(user_id, pile_id, station_id, status, start_time)"
              " VALUES(?, ?, ?, 'charging', ?)");
    q.addBindValue(userId);
    q.addBindValue(pileId);
    q.addBindValue(stationId);
    q.addBindValue(nowString());
    q.exec();
    return q.lastInsertId().toInt();
}

OrderInfo DatabaseManager::getOrderById(int orderId)
{
    OrderInfo o;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("SELECT id, user_id, pile_id, station_id, status, start_time, end_time, kwh, fee FROM charging_order WHERE id = ?");
    q.addBindValue(orderId);
    q.exec();
    if (q.next()) {
        o.id = q.value(0).toInt();
        o.userId = q.value(1).toInt();
        o.pileId = q.value(2).toInt();
        o.stationId = q.value(3).toInt();
        o.status = q.value(4).toString();
        o.startTime = q.value(5).toString();
        o.endTime = q.value(6).toString();
        o.kwh = q.value(7).toDouble();
        o.fee = q.value(8).toDouble();
    }
    return o;
}

void DatabaseManager::finishOrder(int orderId, double kwh, double fee)
{
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("UPDATE charging_order SET status = 'finished', end_time = ?, kwh = ?, fee = ? WHERE id = ?");
    q.addBindValue(nowString());
    q.addBindValue(kwh);
    q.addBindValue(fee);
    q.addBindValue(orderId);
    q.exec();
}

OrderInfo DatabaseManager::getActiveOrder(int userId)
{
    OrderInfo o;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("SELECT id, user_id, pile_id, station_id, status, start_time, end_time, kwh, fee FROM charging_order WHERE user_id = ? AND status = 'charging' ORDER BY id DESC LIMIT 1");
    q.addBindValue(userId);
    q.exec();
    if (q.next()) {
        o.id = q.value(0).toInt();
        o.userId = q.value(1).toInt();
        o.pileId = q.value(2).toInt();
        o.stationId = q.value(3).toInt();
        o.status = q.value(4).toString();
        o.startTime = q.value(5).toString();
        o.endTime = q.value(6).toString();
        o.kwh = q.value(7).toDouble();
        o.fee = q.value(8).toDouble();
    }
    return o;
}

QList<OrderInfo> DatabaseManager::getOrdersByUser(int userId)
{
    QList<OrderInfo> list;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("SELECT id, user_id, pile_id, station_id, status, start_time, end_time, kwh, fee FROM charging_order WHERE user_id = ? ORDER BY id DESC");
    q.addBindValue(userId);
    q.exec();
    while (q.next()) {
        OrderInfo o;
        o.id = q.value(0).toInt();
        o.userId = q.value(1).toInt();
        o.pileId = q.value(2).toInt();
        o.stationId = q.value(3).toInt();
        o.status = q.value(4).toString();
        o.startTime = q.value(5).toString();
        o.endTime = q.value(6).toString();
        o.kwh = q.value(7).toDouble();
        o.fee = q.value(8).toDouble();
        list.append(o);
    }
    return list;
}

QList<OrderInfo> DatabaseManager::getAllOrders()
{
    QList<OrderInfo> list;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.exec("SELECT id, user_id, pile_id, station_id, status, start_time, end_time, kwh, fee FROM charging_order ORDER BY id DESC");
    while (q.next()) {
        OrderInfo o;
        o.id = q.value(0).toInt();
        o.userId = q.value(1).toInt();
        o.pileId = q.value(2).toInt();
        o.stationId = q.value(3).toInt();
        o.status = q.value(4).toString();
        o.startTime = q.value(5).toString();
        o.endTime = q.value(6).toString();
        o.kwh = q.value(7).toDouble();
        o.fee = q.value(8).toDouble();
        list.append(o);
    }
    return list;
}

// ============================================================================
// 统计
// ============================================================================
double DatabaseManager::getRevenueOfDay(const QDate &date)
{
    double sum = 0.0;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("SELECT SUM(fee) FROM charging_order WHERE status = 'finished' AND end_time LIKE ?");
    q.addBindValue(date.toString("yyyy-MM-dd") + "%");
    q.exec();
    if (q.next()) {
        sum = q.value(0).toDouble();
    }
    return sum;
}

double DatabaseManager::getTotalRevenue()
{
    double sum = 0.0;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.exec("SELECT SUM(fee) FROM charging_order WHERE status = 'finished'");
    if (q.next()) {
        sum = q.value(0).toDouble();
    }
    return sum;
}

QList<QPair<QString, double>> DatabaseManager::getRevenueTrend(int days)
{
    QList<QPair<QString, double>> trend;
    for (int i = days - 1; i >= 0; --i) {
        QDate d = QDate::currentDate().addDays(-i);
        trend.append(qMakePair(d.toString("MM-dd"), getRevenueOfDay(d)));
    }
    return trend;
}

QMap<QString, int> DatabaseManager::getPileStatusSummary()
{
    QMap<QString, int> summary;
    summary["idle"] = 0;
    summary["charging"] = 0;
    summary["fault"] = 0;
    summary["offline"] = 0;

    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.exec("SELECT status, COUNT(*) FROM charging_pile GROUP BY status");
    while (q.next()) {
        summary[q.value(0).toString()] = q.value(1).toInt();
    }
    return summary;
}

// ============================================================================
// 管理员
// ============================================================================
bool DatabaseManager::verifyAdmin(const QString &username, const QString &password)
{
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("SELECT password FROM admin WHERE username = ?");
    q.addBindValue(username);
    q.exec();
    if (q.next()) {
        return q.value(0).toString() == hashPassword(password);
    }
    return false;
}