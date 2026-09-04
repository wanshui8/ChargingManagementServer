#include "passwordhasher.h"
#include <QCryptographicHash>

QString PasswordHasher::hash(const QString &password)
{
    QByteArray data = password.toUtf8();
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    return hash.toHex();
}

bool PasswordHasher::verify(const QString &password, const QString &storedHash)
{
    return hash(password) == storedHash;
}
