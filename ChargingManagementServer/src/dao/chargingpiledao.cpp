#include "chargingpiledao.h"
#include "src/db/dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>

static ChargingPile fromQuery(const QSqlQuery& query)
{
    ChargingPile p;
    p.setId(query.value("id").toInt());
    p.setPileSn(query.value("pile_sn").toString());
    p.setStationId(query.value("station_id").toInt());
    p.setPileType(query.value("pile_type").toInt());
    p.setRatedPowerKw(query.value("rated_power_kw").toDouble());
    p.setStatus(query.value("status").toInt());
    p.setTotalChargeCount(query.value("total_charge_count").toInt());
    p.setTotalChargeDuration(query.value("total_charge_duration").toInt());
    p.setCreatedAt(query.value("created_at").toString());
    return p;
}

QList<ChargingPile> ChargingPileDao::findAll()
{
    QList<ChargingPile> list;
    QSqlQuery query(DBManager::instance().getDatabase());
    if (query.exec("SELECT * FROM charging_pile")) {
        while (query.next())
            list.append(fromQuery(query));
    }
    return list;
}

QList<ChargingPile> ChargingPileDao::findByStationId(int stationId)
{
    QList<ChargingPile> list;
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("SELECT * FROM charging_pile WHERE station_id = :stationId");
    query.bindValue(":stationId", stationId);
    if (query.exec()) {
        while (query.next())
            list.append(fromQuery(query));
    }
    return list;
}

std::optional<ChargingPile> ChargingPileDao::findById(int id)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("SELECT * FROM charging_pile WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec() || !query.next())
        return std::nullopt;
    return fromQuery(query);
}

bool ChargingPileDao::insert(const ChargingPile& pile)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("INSERT INTO charging_pile (pile_sn, station_id, pile_type, rated_power_kw, status) "
                  "VALUES (:sn, :stationId, :type, :power, :status)");
    query.bindValue(":sn", pile.pileSn());
    query.bindValue(":stationId", pile.stationId());
    query.bindValue(":type", pile.pileType());
    query.bindValue(":power", pile.ratedPowerKw());
    query.bindValue(":status", pile.status());
    return query.exec();
}

bool ChargingPileDao::updateStatus(int id, int status)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("UPDATE charging_pile SET status = :status WHERE id = :id");
    query.bindValue(":status", status);
    query.bindValue(":id", id);
    return query.exec();
}

bool ChargingPileDao::deleteById(int id)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("DELETE FROM charging_pile WHERE id = :id");
    query.bindValue(":id", id);
    return query.exec();
}
