#include "dbmanager.h"
#include "migrationmanager.h"
#include <QCoreApplication>
#include <QDebug>

DBManager& DBManager::instance()
{
    static DBManager instance;
    return instance;
}

bool DBManager::initDatabase()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");

    QString dbPath = QCoreApplication::applicationDirPath() + "/charging.db";
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qCritical() << "数据库打开失败：" << m_db.lastError().text();
        qCritical() << "   路径：" << dbPath;
        return false;
    }

    qDebug() << "数据库连接成功，文件：" << dbPath;

    if (!MigrationManager::migrate(m_db)) {
        qCritical() << "数据库迁移失败";
        return false;
    }

    return true;
}

QSqlDatabase DBManager::getDatabase()
{
    if (!m_db.isOpen()) {
        m_db.open();
    }
    return m_db;
}
