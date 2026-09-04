#ifndef SYSADMIN_H
#define SYSADMIN_H

#include <QString>

class SysAdmin
{
public:
    SysAdmin() = default;

    SysAdmin(int id, const QString& username, const QString& passwordHash,
             const QString& realName, const QString& phone,
             int role, int status, const QString& createdAt);

    int id() const;
    void setId(int id);

    QString username() const;
    void setUsername(const QString& username);

    QString passwordHash() const;
    void setPasswordHash(const QString& passwordHash);

    QString realName() const;
    void setRealName(const QString& realName);

    QString phone() const;
    void setPhone(const QString& phone);

    int role() const;
    void setRole(int role);

    int status() const;
    void setStatus(int status);

    QString createdAt() const;
    void setCreatedAt(const QString& createdAt);

private:
    int m_id = 0;
    QString m_username;
    QString m_passwordHash;
    QString m_realName;
    QString m_phone;
    int m_role = 0;
    int m_status = 0;
    QString m_createdAt;
};

#endif // SYSADMIN_H
