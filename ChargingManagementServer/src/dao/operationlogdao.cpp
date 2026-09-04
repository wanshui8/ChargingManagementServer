#include "operationlogdao.h"
#include "src/db/dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>

static OperationLog fromQuery(const QSqlQuery& query)
{
    OperationLog l;
    l.setId(query.value("id").toInt());
    l.setAdminId(query.value("admin_id").toInt());
    l.setActionType(query.value("action_type").toString());
    l.setTargetType(query.value("target_type").toString());
    l.setTargetId(query.value("target_id").toInt());
    l.setDetail(query.value("detail").toString());
    l.setIp(query.value("ip").toString());
    l.setCreatedAt(query.value("created_at").toString());
    return l;
}

QList<OperationLog> OperationLogDao::findAll()
{
    QList<OperationLog> list;
    QSqlQuery query(DBManager::instance().getDatabase());
    if (query.exec("SELECT * FROM operation_log")) {
        while (query.next())
            list.append(fromQuery(query));
    }
    return list;
}

bool OperationLogDao::insert(const OperationLog& log)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("INSERT INTO operation_log (admin_id, action_type, target_type, target_id, detail, ip) "
                  "VALUES (:adminId, :actionType, :targetType, :targetId, :detail, :ip)");
    query.bindValue(":adminId", log.adminId());
    query.bindValue(":actionType", log.actionType());
    query.bindValue(":targetType", log.targetType());
    query.bindValue(":targetId", log.targetId());
    query.bindValue(":detail", log.detail());
    query.bindValue(":ip", log.ip());
    return query.exec();
}
