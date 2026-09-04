#include "sysadmin.h"

SysAdmin::SysAdmin(int id, const QString& username, const QString& passwordHash,
                   const QString& realName, const QString& phone,
                   int role, int status, const QString& createdAt)
    : m_id(id), m_username(username), m_passwordHash(passwordHash)
    , m_realName(realName), m_phone(phone), m_role(role)
    , m_status(status), m_createdAt(createdAt)
{}

int SysAdmin::id() const { return m_id; }
void SysAdmin::setId(int id) { m_id = id; }

QString SysAdmin::username() const { return m_username; }
void SysAdmin::setUsername(const QString& username) { m_username = username; }

QString SysAdmin::passwordHash() const { return m_passwordHash; }
void SysAdmin::setPasswordHash(const QString& passwordHash) { m_passwordHash = passwordHash; }

QString SysAdmin::realName() const { return m_realName; }
void SysAdmin::setRealName(const QString& realName) { m_realName = realName; }

QString SysAdmin::phone() const { return m_phone; }
void SysAdmin::setPhone(const QString& phone) { m_phone = phone; }

int SysAdmin::role() const { return m_role; }
void SysAdmin::setRole(int role) { m_role = role; }

int SysAdmin::status() const { return m_status; }
void SysAdmin::setStatus(int status) { m_status = status; }

QString SysAdmin::createdAt() const { return m_createdAt; }
void SysAdmin::setCreatedAt(const QString& createdAt) { m_createdAt = createdAt; }
