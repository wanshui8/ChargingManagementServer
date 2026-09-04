#include "appuserdao.h"
#include "src/db/dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>

static AppUser fromQuery(const QSqlQuery& query)
{
    AppUser u;
    u.setId(query.value("id").toInt());
    u.setPhone(query.value("phone").toString());
    u.setNickname(query.value("nickname").toString());
    u.setPasswordHash(query.value("password_hash").toString());
    u.setWalletBalance(query.value("wallet_balance").toDouble());
    u.setStatus(query.value("status").toInt());
    u.setCreatedAt(query.value("created_at").toString());
    return u;
}

QList<AppUser> AppUserDao::findAll()
{
    QList<AppUser> list;
    QSqlQuery query(DBManager::instance().getDatabase());
    if (query.exec("SELECT * FROM app_user")) {
        while (query.next())
            list.append(fromQuery(query));
    }
    return list;
}

std::optional<AppUser> AppUserDao::findById(int id)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("SELECT * FROM app_user WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec() || !query.next())
        return std::nullopt;
    return fromQuery(query);
}

std::optional<AppUser> AppUserDao::findByPhone(const QString& phone)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("SELECT * FROM app_user WHERE phone = :phone");
    query.bindValue(":phone", phone);
    if (!query.exec() || !query.next())
        return std::nullopt;
    return fromQuery(query);
}

bool AppUserDao::updateStatus(int id, int status)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("UPDATE app_user SET status = :status WHERE id = :id");
    query.bindValue(":status", status);
    query.bindValue(":id", id);
    return query.exec();
}

int AppUserDao::insert(const QString &phone, const QString &nickname, const QString &passwordHash)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("INSERT INTO app_user (phone, nickname, password_hash) "
                  "VALUES (:phone, :nickname, :password_hash)");
    query.bindValue(":phone", phone);
    query.bindValue(":nickname", nickname);
    query.bindValue(":password_hash", passwordHash);
    if (!query.exec())
        return -1;
    return query.lastInsertId().toInt();
}
