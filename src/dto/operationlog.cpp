#include "operationlog.h"

OperationLog::OperationLog(int id, int adminId, const QString& actionType,
                           const QString& targetType, int targetId,
                           const QString& detail, const QString& ip,
                           const QString& createdAt)
    : m_id(id), m_adminId(adminId), m_actionType(actionType)
    , m_targetType(targetType), m_targetId(targetId)
    , m_detail(detail), m_ip(ip), m_createdAt(createdAt)
{}

int OperationLog::id() const { return m_id; }
void OperationLog::setId(int id) { m_id = id; }

int OperationLog::adminId() const { return m_adminId; }
void OperationLog::setAdminId(int adminId) { m_adminId = adminId; }

QString OperationLog::actionType() const { return m_actionType; }
void OperationLog::setActionType(const QString& actionType) { m_actionType = actionType; }

QString OperationLog::targetType() const { return m_targetType; }
void OperationLog::setTargetType(const QString& targetType) { m_targetType = targetType; }

int OperationLog::targetId() const { return m_targetId; }
void OperationLog::setTargetId(int targetId) { m_targetId = targetId; }

QString OperationLog::detail() const { return m_detail; }
void OperationLog::setDetail(const QString& detail) { m_detail = detail; }

QString OperationLog::ip() const { return m_ip; }
void OperationLog::setIp(const QString& ip) { m_ip = ip; }

QString OperationLog::createdAt() const { return m_createdAt; }
void OperationLog::setCreatedAt(const QString& createdAt) { m_createdAt = createdAt; }
