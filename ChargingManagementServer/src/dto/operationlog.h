#ifndef OPERATIONLOG_H
#define OPERATIONLOG_H

#include <QString>

class OperationLog
{
public:
    OperationLog() = default;

    OperationLog(int id, int adminId, const QString& actionType,
                 const QString& targetType, int targetId,
                 const QString& detail, const QString& ip,
                 const QString& createdAt);

    int id() const;
    void setId(int id);

    int adminId() const;
    void setAdminId(int adminId);

    QString actionType() const;
    void setActionType(const QString& actionType);

    QString targetType() const;
    void setTargetType(const QString& targetType);

    int targetId() const;
    void setTargetId(int targetId);

    QString detail() const;
    void setDetail(const QString& detail);

    QString ip() const;
    void setIp(const QString& ip);

    QString createdAt() const;
    void setCreatedAt(const QString& createdAt);

private:
    int m_id = 0;
    int m_adminId = 0;
    QString m_actionType;
    QString m_targetType;
    int m_targetId = 0;
    QString m_detail;
    QString m_ip;
    QString m_createdAt;
};

#endif // OPERATIONLOG_H
