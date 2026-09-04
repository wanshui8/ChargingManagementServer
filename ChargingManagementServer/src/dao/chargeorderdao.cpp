#include "chargeorderdao.h"
#include "src/db/dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>

static ChargeOrder fromQuery(const QSqlQuery& query)
{
    ChargeOrder o;
    o.setId(query.value("id").toInt());
    o.setOrderNo(query.value("order_no").toString());
    o.setUserId(query.value("user_id").toInt());
    o.setPileId(query.value("pile_id").toInt());
    o.setStartTime(query.value("start_time").toString());
    o.setEndTime(query.value("end_time").toString());
    o.setTotalDuration(query.value("total_duration").toInt());
    o.setTotalEnergyKwh(query.value("total_energy_kwh").toDouble());
    o.setElectricityFee(query.value("electricity_fee").toDouble());
    o.setServiceFee(query.value("service_fee").toDouble());
    o.setOccupancyFee(query.value("occupancy_fee").toDouble());
    o.setTotalAmount(query.value("total_amount").toDouble());
    o.setOrderStatus(query.value("order_status").toInt());
    o.setCreatedAt(query.value("created_at").toString());
    return o;
}

QList<ChargeOrder> ChargeOrderDao::findAll()
{
    QList<ChargeOrder> list;
    QSqlQuery query(DBManager::instance().getDatabase());
    if (query.exec("SELECT * FROM charge_order")) {
        while (query.next())
            list.append(fromQuery(query));
    }
    return list;
}

QList<ChargeOrder> ChargeOrderDao::findByUserId(int userId)
{
    QList<ChargeOrder> list;
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("SELECT * FROM charge_order WHERE user_id = :userId");
    query.bindValue(":userId", userId);
    if (query.exec()) {
        while (query.next())
            list.append(fromQuery(query));
    }
    return list;
}

std::optional<ChargeOrder> ChargeOrderDao::findById(int id)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("SELECT * FROM charge_order WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec() || !query.next())
        return std::nullopt;
    return fromQuery(query);
}

bool ChargeOrderDao::insert(const ChargeOrder& order)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("INSERT INTO charge_order (order_no, user_id, pile_id, start_time, end_time, "
                  "total_duration, total_energy_kwh, electricity_fee, service_fee, occupancy_fee, "
                  "total_amount, order_status) "
                  "VALUES (:orderNo, :userId, :pileId, :startTime, :endTime, "
                  ":duration, :energy, :elecFee, :svcFee, :occFee, :total, :status)");
    query.bindValue(":orderNo", order.orderNo());
    query.bindValue(":userId", order.userId());
    query.bindValue(":pileId", order.pileId());
    query.bindValue(":startTime", order.startTime());
    query.bindValue(":endTime", order.endTime());
    query.bindValue(":duration", order.totalDuration());
    query.bindValue(":energy", order.totalEnergyKwh());
    query.bindValue(":elecFee", order.electricityFee());
    query.bindValue(":svcFee", order.serviceFee());
    query.bindValue(":occFee", order.occupancyFee());
    query.bindValue(":total", order.totalAmount());
    query.bindValue(":status", order.orderStatus());
    return query.exec();
}

bool ChargeOrderDao::updateStatus(int id, int status)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("UPDATE charge_order SET order_status = :status WHERE id = :id");
    query.bindValue(":status", status);
    query.bindValue(":id", id);
    return query.exec();
}
