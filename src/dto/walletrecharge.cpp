#include "walletrecharge.h"

WalletRecharge::WalletRecharge(int id, int userId, double amount,
                               int payMethod, const QString& orderNo,
                               int status, const QString& createdAt)
    : m_id(id), m_userId(userId), m_amount(amount)
    , m_payMethod(payMethod), m_orderNo(orderNo)
    , m_status(status), m_createdAt(createdAt)
{}

int WalletRecharge::id() const { return m_id; }
void WalletRecharge::setId(int id) { m_id = id; }

int WalletRecharge::userId() const { return m_userId; }
void WalletRecharge::setUserId(int userId) { m_userId = userId; }

double WalletRecharge::amount() const { return m_amount; }
void WalletRecharge::setAmount(double amount) { m_amount = amount; }

int WalletRecharge::payMethod() const { return m_payMethod; }
void WalletRecharge::setPayMethod(int payMethod) { m_payMethod = payMethod; }

QString WalletRecharge::orderNo() const { return m_orderNo; }
void WalletRecharge::setOrderNo(const QString& orderNo) { m_orderNo = orderNo; }

int WalletRecharge::status() const { return m_status; }
void WalletRecharge::setStatus(int status) { m_status = status; }

QString WalletRecharge::createdAt() const { return m_createdAt; }
void WalletRecharge::setCreatedAt(const QString& createdAt) { m_createdAt = createdAt; }
