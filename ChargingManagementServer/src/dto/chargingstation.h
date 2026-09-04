#ifndef CHARGINGSTATION_H
#define CHARGINGSTATION_H

#include <QString>

class ChargingStation
{
public:
    ChargingStation() = default;

    ChargingStation(int id, const QString& stationName, const QString& address,
                    double longitude, double latitude, int totalPiles,
                    int onlineCount, int status, const QString& createdAt);

    int id() const;
    void setId(int id);

    QString stationName() const;
    void setStationName(const QString& stationName);

    QString address() const;
    void setAddress(const QString& address);

    double longitude() const;
    void setLongitude(double longitude);

    double latitude() const;
    void setLatitude(double latitude);

    int totalPiles() const;
    void setTotalPiles(int totalPiles);

    int onlineCount() const;
    void setOnlineCount(int onlineCount);

    int status() const;
    void setStatus(int status);

    QString createdAt() const;
    void setCreatedAt(const QString& createdAt);

private:
    int m_id = 0;
    QString m_stationName;
    QString m_address;
    double m_longitude = 0.0;
    double m_latitude = 0.0;
    int m_totalPiles = 0;
    int m_onlineCount = 0;
    int m_status = 0;
    QString m_createdAt;
};

#endif // CHARGINGSTATION_H
