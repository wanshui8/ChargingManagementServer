#ifndef APPUSER_H
#define APPUSER_H

#include <QString>

class AppUser
{
public:
    AppUser() = default;

    AppUser(int id, const QString& phone, const QString& nickname,
            const QString& passwordHash, double walletBalance,
            int status, const QString& createdAt);

    int id() const;
    void setId(int id);

    QString phone() const;
    void setPhone(const QString& phone);

    QString nickname() const;
    void setNickname(const QString& nickname);

    QString passwordHash() const;
    void setPasswordHash(const QString& passwordHash);

    double walletBalance() const;
    void setWalletBalance(double walletBalance);

    int status() const;
    void setStatus(int status);

    QString createdAt() const;
    void setCreatedAt(const QString& createdAt);

private:
    int m_id = 0;
    QString m_phone;
    QString m_nickname;
    QString m_passwordHash;
    double m_walletBalance = 0.0;
    int m_status = 0;
    QString m_createdAt;
};

#endif // APPUSER_H
