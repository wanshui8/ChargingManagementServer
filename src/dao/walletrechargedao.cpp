#include "walletrechargedao.h"
#include "src/db/dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>

static WalletRecharge fromQuery(const QSqlQuery& query)
{
    WalletRecharge r;
    r.setId(query.value("id").toInt());
    r.setUserId(query.value("user_id").toInt());
    r.setAmount(query.value("amount").toDouble());
    r.setPayMethod(query.value("pay_method").toInt());
    r.setOrderNo(query.value("order_no").toString());
    r.setStatus(query.value("status").toInt());
    r.setCreatedAt(query.value("created_at").toString());
    return r;
}

QList<WalletRecharge> WalletRechargeDao::findAll()
{
    QList<WalletRecharge> list;
    QSqlQuery query(DBManager::instance().getDatabase());
    if (query.exec("SELECT * FROM wallet_recharge")) {
        while (query.next())
            list.append(fromQuery(query));
    }
    return list;
}

QList<WalletRecharge> WalletRechargeDao::findByUserId(int userId)
{
    QList<WalletRecharge> list;
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("SELECT * FROM wallet_recharge WHERE user_id = :userId");
    query.bindValue(":userId", userId);
    if (query.exec()) {
        while (query.next())
            list.append(fromQuery(query));
    }
    return list;
}

bool WalletRechargeDao::insert(const WalletRecharge& recharge)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("INSERT INTO wallet_recharge (user_id, amount, pay_method, order_no, status) "
                  "VALUES (:userId, :amount, :payMethod, :orderNo, :status)");
    query.bindValue(":userId", recharge.userId());
    query.bindValue(":amount", recharge.amount());
    query.bindValue(":payMethod", recharge.payMethod());
    query.bindValue(":orderNo", recharge.orderNo());
    query.bindValue(":status", recharge.status());
    return query.exec();
}
