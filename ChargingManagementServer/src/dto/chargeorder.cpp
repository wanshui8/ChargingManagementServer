#include "chargeorder.h"

ChargeOrder::ChargeOrder(int id, const QString& orderNo, int userId, int pileId,
                         const QString& startTime, const QString& endTime,
                         int totalDuration, double totalEnergyKwh,
                         double electricityFee, double serviceFee,
                         double occupancyFee, double totalAmount,
                         int orderStatus, const QString& createdAt)
    : m_id(id), m_orderNo(orderNo), m_userId(userId), m_pileId(pileId)
    , m_startTime(startTime), m_endTime(endTime), m_totalDuration(totalDuration)
    , m_totalEnergyKwh(totalEnergyKwh), m_electricityFee(electricityFee)
    , m_serviceFee(serviceFee), m_occupancyFee(occupancyFee)
    , m_totalAmount(totalAmount), m_orderStatus(orderStatus)
    , m_createdAt(createdAt)
{}

int ChargeOrder::id() const { return m_id; }
void ChargeOrder::setId(int id) { m_id = id; }

QString ChargeOrder::orderNo() const { return m_orderNo; }
void ChargeOrder::setOrderNo(const QString& orderNo) { m_orderNo = orderNo; }

int ChargeOrder::userId() const { return m_userId; }
void ChargeOrder::setUserId(int userId) { m_userId = userId; }

int ChargeOrder::pileId() const { return m_pileId; }
void ChargeOrder::setPileId(int pileId) { m_pileId = pileId; }

QString ChargeOrder::startTime() const { return m_startTime; }
void ChargeOrder::setStartTime(const QString& startTime) { m_startTime = startTime; }

QString ChargeOrder::endTime() const { return m_endTime; }
void ChargeOrder::setEndTime(const QString& endTime) { m_endTime = endTime; }

int ChargeOrder::totalDuration() const { return m_totalDuration; }
void ChargeOrder::setTotalDuration(int totalDuration) { m_totalDuration = totalDuration; }

double ChargeOrder::totalEnergyKwh() const { return m_totalEnergyKwh; }
void ChargeOrder::setTotalEnergyKwh(double totalEnergyKwh) { m_totalEnergyKwh = totalEnergyKwh; }

double ChargeOrder::electricityFee() const { return m_electricityFee; }
void ChargeOrder::setElectricityFee(double electricityFee) { m_electricityFee = electricityFee; }

double ChargeOrder::serviceFee() const { return m_serviceFee; }
void ChargeOrder::setServiceFee(double serviceFee) { m_serviceFee = serviceFee; }

double ChargeOrder::occupancyFee() const { return m_occupancyFee; }
void ChargeOrder::setOccupancyFee(double occupancyFee) { m_occupancyFee = occupancyFee; }

double ChargeOrder::totalAmount() const { return m_totalAmount; }
void ChargeOrder::setTotalAmount(double totalAmount) { m_totalAmount = totalAmount; }

int ChargeOrder::orderStatus() const { return m_orderStatus; }
void ChargeOrder::setOrderStatus(int orderStatus) { m_orderStatus = orderStatus; }

QString ChargeOrder::createdAt() const { return m_createdAt; }
void ChargeOrder::setCreatedAt(const QString& createdAt) { m_createdAt = createdAt; }
