#ifndef CHARGEORDER_H
#define CHARGEORDER_H

#include <QString>

class ChargeOrder
{
public:
    ChargeOrder() = default;

    ChargeOrder(int id, const QString& orderNo, int userId, int pileId,
                const QString& startTime, const QString& endTime,
                int totalDuration, double totalEnergyKwh,
                double electricityFee, double serviceFee,
                double occupancyFee, double totalAmount,
                int orderStatus, const QString& createdAt);

    int id() const;
    void setId(int id);

    QString orderNo() const;
    void setOrderNo(const QString& orderNo);

    int userId() const;
    void setUserId(int userId);

    int pileId() const;
    void setPileId(int pileId);

    QString startTime() const;
    void setStartTime(const QString& startTime);

    QString endTime() const;
    void setEndTime(const QString& endTime);

    int totalDuration() const;
    void setTotalDuration(int totalDuration);

    double totalEnergyKwh() const;
    void setTotalEnergyKwh(double totalEnergyKwh);

    double electricityFee() const;
    void setElectricityFee(double electricityFee);

    double serviceFee() const;
    void setServiceFee(double serviceFee);

    double occupancyFee() const;
    void setOccupancyFee(double occupancyFee);

    double totalAmount() const;
    void setTotalAmount(double totalAmount);

    int orderStatus() const;
    void setOrderStatus(int orderStatus);

    QString createdAt() const;
    void setCreatedAt(const QString& createdAt);

private:
    int m_id = 0;
    QString m_orderNo;
    int m_userId = 0;
    int m_pileId = 0;
    QString m_startTime;
    QString m_endTime;
    int m_totalDuration = 0;
    double m_totalEnergyKwh = 0.0;
    double m_electricityFee = 0.0;
    double m_serviceFee = 0.0;
    double m_occupancyFee = 0.0;
    double m_totalAmount = 0.0;
    int m_orderStatus = 0;
    QString m_createdAt;
};

#endif // CHARGEORDER_H
