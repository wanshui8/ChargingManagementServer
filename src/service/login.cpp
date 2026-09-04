#include "login.h"
#include "src/dao/sysadmindao.h"
#include "passwordhasher.h"

std::optional<SysAdmin> LoginService::login(const QString& username, const QString& password)
{
    auto adminOpt = SysAdminDao::findByUsername(username);
    if (!adminOpt.has_value())
        return std::nullopt;

    const SysAdmin& admin = adminOpt.value();
    if (!PasswordHasher::verify(password, admin.passwordHash()))
        return std::nullopt;

    if (admin.status() != 1)
        return std::nullopt;

    return admin;
}
