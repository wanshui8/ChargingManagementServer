#ifndef MIGRATIONMANAGER_H
#define MIGRATIONMANAGER_H

#include <QSqlDatabase>

class MigrationManager
{
public:
    static bool migrate(QSqlDatabase& db);

private:
    static bool migrateToV1(QSqlDatabase& db);
};

#endif // MIGRATIONMANAGER_H
