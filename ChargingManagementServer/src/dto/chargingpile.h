#ifndef CHARGINGPILE_H
#define CHARGINGPILE_H

#include <QString>

class ChargingPile
{
public:
    ChargingPile() = default;

    ChargingPile(int id, const QString& pileSn, int stationId,
                 int pileType, double ratedPowerKw, int status,
                 int totalChargeCount, int totalChargeDuration,
                 const QString& createdAt);

    int id() const;
    void setId(int id);

    QString pileSn() const;
    void setPileSn(const QString& pileSn);

    int stationId() const;
    void setStationId(int stationId);

    int pileType() const;
    void setPileType(int pileType);

    double ratedPowerKw() const;
    void setRatedPowerKw(double ratedPowerKw);

    int status() const;
    void setStatus(int status);

    int totalChargeCount() const;
    void setTotalChargeCount(int totalChargeCount);

    int totalChargeDuration() const;
    void setTotalChargeDuration(int totalChargeDuration);

    QString createdAt() const;
    void setCreatedAt(const QString& createdAt);

private:
    int m_id = 0;
    QString m_pileSn;
    int m_stationId = 0;
    int m_pileType = 0;
    double m_ratedPowerKw = 0.0;
    int m_status = 0;
    int m_totalChargeCount = 0;
    int m_totalChargeDuration = 0;
    QString m_createdAt;
};

#endif // CHARGINGPILE_H
