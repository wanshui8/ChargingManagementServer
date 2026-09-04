#ifndef OPERATIONLOGDAO_H
#define OPERATIONLOGDAO_H

#include "src/dto/operationlog.h"
#include <QList>

class OperationLogDao
{
public:
    static QList<OperationLog> findAll();
    static bool insert(const OperationLog& log);
};

#endif // OPERATIONLOGDAO_H
