#ifndef PASSWORDHASHER_H
#define PASSWORDHASHER_H

#include <QString>

class PasswordHasher
{
public:
    static QString hash(const QString &password);
    static bool verify(const QString &password, const QString &storedHash);
};

#endif // PASSWORDHASHER_H
