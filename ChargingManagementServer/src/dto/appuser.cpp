#include "appuser.h"

AppUser::AppUser(int id, const QString& phone, const QString& nickname,
                 const QString& passwordHash, double walletBalance,
                 int status, const QString& createdAt)
    : m_id(id), m_phone(phone), m_nickname(nickname)
    , m_passwordHash(passwordHash), m_walletBalance(walletBalance)
    , m_status(status), m_createdAt(createdAt)
{}

int AppUser::id() const { return m_id; }
void AppUser::setId(int id) { m_id = id; }

QString AppUser::phone() const { return m_phone; }
void AppUser::setPhone(const QString& phone) { m_phone = phone; }

QString AppUser::nickname() const { return m_nickname; }
void AppUser::setNickname(const QString& nickname) { m_nickname = nickname; }

QString AppUser::passwordHash() const { return m_passwordHash; }
void AppUser::setPasswordHash(const QString& passwordHash) { m_passwordHash = passwordHash; }

double AppUser::walletBalance() const { return m_walletBalance; }
void AppUser::setWalletBalance(double walletBalance) { m_walletBalance = walletBalance; }

int AppUser::status() const { return m_status; }
void AppUser::setStatus(int status) { m_status = status; }

QString AppUser::createdAt() const { return m_createdAt; }
void AppUser::setCreatedAt(const QString& createdAt) { m_createdAt = createdAt; }
