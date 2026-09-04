#include "chargingstationdao.h"
#include "src/db/dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>

static ChargingStation fromQuery(const QSqlQuery& query)
{
    ChargingStation s;
    s.setId(query.value("id").toInt());
    s.setStationName(query.value("station_name").toString());
    s.setAddress(query.value("address").toString());
    s.setLongitude(query.value("longitude").toDouble());
    s.setLatitude(query.value("latitude").toDouble());
    s.setTotalPiles(query.value("total_piles").toInt());
    s.setOnlineCount(query.value("online_count").toInt());
    s.setStatus(query.value("status").toInt());
    s.setCreatedAt(query.value("created_at").toString());
    return s;
}

QList<ChargingStation> ChargingStationDao::findAll()
{
    QList<ChargingStation> list;
    QSqlQuery query(DBManager::instance().getDatabase());
    if (query.exec("SELECT * FROM charging_station")) {
        while (query.next())
            list.append(fromQuery(query));
    }
    return list;
}

std::optional<ChargingStation> ChargingStationDao::findById(int id)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("SELECT * FROM charging_station WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec() || !query.next())
        return std::nullopt;
    return fromQuery(query);
}

bool ChargingStationDao::insert(const ChargingStation& station)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("INSERT INTO charging_station (station_name, address, longitude, latitude, total_piles, online_count, status) "
                  "VALUES (:name, :addr, :lng, :lat, :total, :online, :status)");
    query.bindValue(":name", station.stationName());
    query.bindValue(":addr", station.address());
    query.bindValue(":lng", station.longitude());
    query.bindValue(":lat", station.latitude());
    query.bindValue(":total", station.totalPiles());
    query.bindValue(":online", station.onlineCount());
    query.bindValue(":status", station.status());
    return query.exec();
}

bool ChargingStationDao::update(const ChargingStation& station)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("UPDATE charging_station SET station_name=:name, address=:addr, longitude=:lng, latitude=:lat, "
                  "total_piles=:total, online_count=:online, status=:status WHERE id=:id");
    query.bindValue(":name", station.stationName());
    query.bindValue(":addr", station.address());
    query.bindValue(":lng", station.longitude());
    query.bindValue(":lat", station.latitude());
    query.bindValue(":total", station.totalPiles());
    query.bindValue(":online", station.onlineCount());
    query.bindValue(":status", station.status());
    query.bindValue(":id", station.id());
    return query.exec();
}

bool ChargingStationDao::deleteById(int id)
{
    QSqlQuery query(DBManager::instance().getDatabase());
    query.prepare("DELETE FROM charging_station WHERE id = :id");
    query.bindValue(":id", id);
    return query.exec();
}
