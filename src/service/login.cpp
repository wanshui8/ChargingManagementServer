#include "login.h"
#include "src/dao/sysadmindao.h"

std::optional<SysAdmin> LoginService::login(const QString& username, const QString& password)
{
    auto adminOpt = SysAdminDao::findByUsername(username);
    if (!adminOpt.has_value())
        return std::nullopt;

    const SysAdmin& admin = adminOpt.value();
    if (admin.passwordHash() != password)
        return std::nullopt;

    if (admin.status() != 1)
        return std::nullopt;

    return admin;
}
