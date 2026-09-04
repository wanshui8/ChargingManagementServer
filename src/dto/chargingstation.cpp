#include "chargingstation.h"

ChargingStation::ChargingStation(int id, const QString& stationName, const QString& address,
                                 double longitude, double latitude, int totalPiles,
                                 int onlineCount, int status, const QString& createdAt)
    : m_id(id), m_stationName(stationName), m_address(address)
    , m_longitude(longitude), m_latitude(latitude), m_totalPiles(totalPiles)
    , m_onlineCount(onlineCount), m_status(status), m_createdAt(createdAt)
{}

int ChargingStation::id() const { return m_id; }
void ChargingStation::setId(int id) { m_id = id; }

QString ChargingStation::stationName() const { return m_stationName; }
void ChargingStation::setStationName(const QString& stationName) { m_stationName = stationName; }

QString ChargingStation::address() const { return m_address; }
void ChargingStation::setAddress(const QString& address) { m_address = address; }

double ChargingStation::longitude() const { return m_longitude; }
void ChargingStation::setLongitude(double longitude) { m_longitude = longitude; }

double ChargingStation::latitude() const { return m_latitude; }
void ChargingStation::setLatitude(double latitude) { m_latitude = latitude; }

int ChargingStation::totalPiles() const { return m_totalPiles; }
void ChargingStation::setTotalPiles(int totalPiles) { m_totalPiles = totalPiles; }

int ChargingStation::onlineCount() const { return m_onlineCount; }
void ChargingStation::setOnlineCount(int onlineCount) { m_onlineCount = onlineCount; }

int ChargingStation::status() const { return m_status; }
void ChargingStation::setStatus(int status) { m_status = status; }

QString ChargingStation::createdAt() const { return m_createdAt; }
void ChargingStation::setCreatedAt(const QString& createdAt) { m_createdAt = createdAt; }
