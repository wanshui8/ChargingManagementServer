#include "chargingpile.h"

ChargingPile::ChargingPile(int id, const QString& pileSn, int stationId,
                           int pileType, double ratedPowerKw, int status,
                           int totalChargeCount, int totalChargeDuration,
                           const QString& createdAt)
    : m_id(id), m_pileSn(pileSn), m_stationId(stationId), m_pileType(pileType)
    , m_ratedPowerKw(ratedPowerKw), m_status(status)
    , m_totalChargeCount(totalChargeCount), m_totalChargeDuration(totalChargeDuration)
    , m_createdAt(createdAt)
{}

int ChargingPile::id() const { return m_id; }
void ChargingPile::setId(int id) { m_id = id; }

QString ChargingPile::pileSn() const { return m_pileSn; }
void ChargingPile::setPileSn(const QString& pileSn) { m_pileSn = pileSn; }

int ChargingPile::stationId() const { return m_stationId; }
void ChargingPile::setStationId(int stationId) { m_stationId = stationId; }

int ChargingPile::pileType() const { return m_pileType; }
void ChargingPile::setPileType(int pileType) { m_pileType = pileType; }

double ChargingPile::ratedPowerKw() const { return m_ratedPowerKw; }
void ChargingPile::setRatedPowerKw(double ratedPowerKw) { m_ratedPowerKw = ratedPowerKw; }

int ChargingPile::status() const { return m_status; }
void ChargingPile::setStatus(int status) { m_status = status; }

int ChargingPile::totalChargeCount() const { return m_totalChargeCount; }
void ChargingPile::setTotalChargeCount(int totalChargeCount) { m_totalChargeCount = totalChargeCount; }

int ChargingPile::totalChargeDuration() const { return m_totalChargeDuration; }
void ChargingPile::setTotalChargeDuration(int totalChargeDuration) { m_totalChargeDuration = totalChargeDuration; }

QString ChargingPile::createdAt() const { return m_createdAt; }
void ChargingPile::setCreatedAt(const QString& createdAt) { m_createdAt = createdAt; }
