#ifndef SYSADMINDAO_H
#define SYSADMINDAO_H

#include "src/dto/sysadmin.h"
#include <optional>
#include <QString>

class SysAdminDao
{
public:
    static std::optional<SysAdmin> findByUsername(const QString& username);
    static std::optional<SysAdmin> findById(int id);
    static bool updateStatus(int id, int status);
};

#endif // SYSADMINDAO_H
