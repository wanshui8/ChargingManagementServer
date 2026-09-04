#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

class DBManager
{
public:
    // 单例
    static DBManager& instance();

    // 初始化数据库（连接、建表、版本升级）
    bool initDatabase();

    // 获取数据库连接（供 DAO 使用）
    QSqlDatabase getDatabase();

private:
    DBManager() = default;
    ~DBManager() = default;
    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;

    QSqlDatabase m_db;
};

#endif // DBMANAGER_H
