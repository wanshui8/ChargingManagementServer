#ifndef APPUSERDAO_H
#define APPUSERDAO_H

#include "src/dto/appuser.h"
#include <QList>
#include <optional>

class AppUserDao
{
public:
    static QList<AppUser> findAll();
    static std::optional<AppUser> findById(int id);
    static std::optional<AppUser> findByPhone(const QString& phone);
    static bool updateStatus(int id, int status);
    static int insert(const QString &phone, const QString &nickname, const QString &passwordHash);
};

#endif // APPUSERDAO_H
