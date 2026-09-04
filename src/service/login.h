#ifndef LOGIN_H
#define LOGIN_H

#include "src/dto/sysadmin.h"
#include <optional>
#include <QString>

class LoginService
{
public:
    static std::optional<SysAdmin> login(const QString& username, const QString& password);
};

#endif // LOGIN_H
