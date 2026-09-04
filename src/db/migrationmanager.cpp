#include "migrationmanager.h"
#include "src/service/passwordhasher.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

bool MigrationManager::migrate(QSqlDatabase& db)
{
    QSqlQuery query(db);
    query.exec("PRAGMA user_version;");
    query.next();
    int currentVersion = query.value(0).toInt();
    qDebug() << "当前数据库版本：" << currentVersion;

    if (currentVersion < 1) {
        if (!migrateToV1(db)) return false;
    }

    if (currentVersion < 2) {
        if (!migrateToV2(db)) return false;
    }

    return true;
}

// ---------- V1: 创建所有初始表 + 默认管理员 ----------

bool MigrationManager::migrateToV1(QSqlDatabase& db)
{
    QSqlQuery query(db);
    qDebug() << "执行迁移：版本 0 -> 1 (创建初始表)";

    QStringList sqls;

    // 1. 系统管理员表
    sqls << "CREATE TABLE IF NOT EXISTS sys_admin ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "username TEXT NOT NULL UNIQUE, "
            "password_hash TEXT NOT NULL, "
            "real_name TEXT, "
            "phone TEXT, "
            "role INTEGER DEFAULT 0, "
            "status INTEGER DEFAULT 1, "
            "created_at TEXT DEFAULT (datetime('now','localtime')))";

    // 2. 充电站表
    sqls << "CREATE TABLE IF NOT EXISTS charging_station ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "station_name TEXT NOT NULL, "
            "address TEXT, "
            "longitude REAL DEFAULT 0, "
            "latitude REAL DEFAULT 0, "
            "total_piles INTEGER DEFAULT 0, "
            "online_count INTEGER DEFAULT 0, "
            "status INTEGER DEFAULT 1, "
            "created_at TEXT DEFAULT (datetime('now','localtime')))";

    // 3. 充电桩表
    sqls << "CREATE TABLE IF NOT EXISTS charging_pile ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "pile_sn TEXT NOT NULL UNIQUE, "
            "station_id INTEGER NOT NULL, "
            "pile_type INTEGER DEFAULT 0, "
            "rated_power_kw REAL DEFAULT 0, "
            "status INTEGER DEFAULT 0, "
            "total_charge_count INTEGER DEFAULT 0, "
            "total_charge_duration INTEGER DEFAULT 0, "
            "created_at TEXT DEFAULT (datetime('now','localtime')), "
            "FOREIGN KEY (station_id) REFERENCES charging_station(id))";

    // 4. 用户表
    sqls << "CREATE TABLE IF NOT EXISTS app_user ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "phone TEXT NOT NULL UNIQUE, "
            "nickname TEXT, "
            "password_hash TEXT NOT NULL, "
            "wallet_balance REAL DEFAULT 0, "
            "status INTEGER DEFAULT 1, "
            "created_at TEXT DEFAULT (datetime('now','localtime')))";

    // 5. 充电订单表
    sqls << "CREATE TABLE IF NOT EXISTS charge_order ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "order_no TEXT NOT NULL UNIQUE, "
            "user_id INTEGER NOT NULL, "
            "pile_id INTEGER NOT NULL, "
            "start_time TEXT, "
            "end_time TEXT, "
            "total_duration INTEGER DEFAULT 0, "
            "total_energy_kwh REAL DEFAULT 0, "
            "electricity_fee REAL DEFAULT 0, "
            "service_fee REAL DEFAULT 0, "
            "occupancy_fee REAL DEFAULT 0, "
            "total_amount REAL DEFAULT 0, "
            "order_status INTEGER DEFAULT 0, "
            "created_at TEXT DEFAULT (datetime('now','localtime')), "
            "FOREIGN KEY (user_id) REFERENCES app_user(id), "
            "FOREIGN KEY (pile_id) REFERENCES charging_pile(id))";

    // 6. 充值记录表
    sqls << "CREATE TABLE IF NOT EXISTS wallet_recharge ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "user_id INTEGER NOT NULL, "
            "amount REAL NOT NULL, "
            "pay_method INTEGER DEFAULT 0, "
            "order_no TEXT NOT NULL, "
            "status INTEGER DEFAULT 0, "
            "created_at TEXT DEFAULT (datetime('now','localtime')), "
            "FOREIGN KEY (user_id) REFERENCES app_user(id))";

    // 7. 操作日志表
    sqls << "CREATE TABLE IF NOT EXISTS operation_log ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "admin_id INTEGER NOT NULL, "
            "action_type TEXT NOT NULL, "
            "target_type TEXT, "
            "target_id INTEGER DEFAULT 0, "
            "detail TEXT, "
            "ip TEXT, "
            "created_at TEXT DEFAULT (datetime('now','localtime')))";

    for (const QString& sql : sqls) {
        if (!query.exec(sql)) {
            qCritical() << "V1 建表失败：" << query.lastError().text();
            qCritical() << "SQL：" << sql;
            return false;
        }
    }

    // 插入默认管理员 admin / 123456（明文存储，后续改为哈希）
    // 此处仅作测试使用！后续必须删除！所有使用了这一版本的数据库最后都需要删除！
    QString insertAdmin = "INSERT OR IGNORE INTO sys_admin (username, password_hash, real_name, role, status) "
                          "VALUES ('admin', '123456', '超级管理员', 1, 1)";
    if (!query.exec(insertAdmin)) {
        qCritical() << "V1 插入默认管理员失败：" << query.lastError().text();
        return false;
    }

    query.exec("PRAGMA user_version = 1;");
    qDebug() << "V1 迁移完成，共创建 7 张表 + 默认管理员";
    return true;
}

// ---------- V2: 管理员密码加密 + 新增示例账户 ----------

bool MigrationManager::migrateToV2(QSqlDatabase& db)
{
    QSqlQuery query(db);
    qDebug() << "执行迁移：版本 1 -> 2 (管理员密码加密)";

    // 将已有 admin 账户的明文密码转为 SHA-256 哈希
    QString adminHash = PasswordHasher::hash("123456");
    query.prepare("UPDATE sys_admin SET password_hash = :hash WHERE username = 'admin'");
    query.bindValue(":hash", adminHash);
    if (!query.exec()) {
        qCritical() << "V2 更新 admin 密码失败：" << query.lastError().text();
        return false;
    }

    // 插入示例管理员账户 ad，密码 111111（哈希存储）
    QString adHash = PasswordHasher::hash("111111");
    query.prepare("INSERT OR IGNORE INTO sys_admin (username, password_hash, real_name, role, status) "
                  "VALUES (:username, :hash, :real_name, :role, :status)");
    query.bindValue(":username", "ad");
    query.bindValue(":hash", adHash);
    query.bindValue(":real_name", "示例管理员");
    query.bindValue(":role", 0);
    query.bindValue(":status", 1);
    if (!query.exec()) {
        qCritical() << "V2 插入示例管理员 ad 失败：" << query.lastError().text();
        return false;
    }

    query.exec("PRAGMA user_version = 2;");
    qDebug() << "V2 迁移完成，admin 密码已加密，新增示例账户 ad";
    return true;
}
