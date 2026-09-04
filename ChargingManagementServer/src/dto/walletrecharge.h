#ifndef WALLETRECHARGE_H
#define WALLETRECHARGE_H

#include <QString>

class WalletRecharge
{
public:
    WalletRecharge() = default;

    WalletRecharge(int id, int userId, double amount,
                   int payMethod, const QString& orderNo,
                   int status, const QString& createdAt);

    int id() const;
    void setId(int id);

    int userId() const;
    void setUserId(int userId);

    double amount() const;
    void setAmount(double amount);

    int payMethod() const;
    void setPayMethod(int payMethod);

    QString orderNo() const;
    void setOrderNo(const QString& orderNo);

    int status() const;
    void setStatus(int status);

    QString createdAt() const;
    void setCreatedAt(const QString& createdAt);

private:
    int m_id = 0;
    int m_userId = 0;
    double m_amount = 0.0;
    int m_payMethod = 0;
    QString m_orderNo;
    int m_status = 0;
    QString m_createdAt;
};

#endif // WALLETRECHARGE_H
