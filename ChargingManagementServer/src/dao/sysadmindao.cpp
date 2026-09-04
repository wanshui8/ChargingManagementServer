#include "sysadmindao.h"
#include "src/db/dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>

std::optional<SysAdmin> SysAdminDao::findByUsername(const QString& username)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("SELECT * FROM sys_admin WHERE username = :username");
    query.bindValue(":username", username);
    if (!query.exec() || !query.next())
        return std::nullopt;

    SysAdmin admin;
    admin.setId(query.value("id").toInt());
    admin.setUsername(query.value("username").toString());
    admin.setPasswordHash(query.value("password_hash").toString());
    admin.setRealName(query.value("real_name").toString());
    admin.setPhone(query.value("phone").toString());
    admin.setRole(query.value("role").toInt());
    admin.setStatus(query.value("status").toInt());
    admin.setCreatedAt(query.value("created_at").toString());
    return admin;
}

std::optional<SysAdmin> SysAdminDao::findById(int id)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("SELECT * FROM sys_admin WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec() || !query.next())
        return std::nullopt;

    SysAdmin admin;
    admin.setId(query.value("id").toInt());
    admin.setUsername(query.value("username").toString());
    admin.setPasswordHash(query.value("password_hash").toString());
    admin.setRealName(query.value("real_name").toString());
    admin.setPhone(query.value("phone").toString());
    admin.setRole(query.value("role").toInt());
    admin.setStatus(query.value("status").toInt());
    admin.setCreatedAt(query.value("created_at").toString());
    return admin;
}

bool SysAdminDao::updateStatus(int id, int status)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("UPDATE sys_admin SET status = :status WHERE id = :id");
    query.bindValue(":status", status);
    query.bindValue(":id", id);
    return query.exec();
}
