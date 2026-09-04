#ifndef CHARGINGSTATIONDAO_H
#define CHARGINGSTATIONDAO_H

#include "src/dto/chargingstation.h"
#include <QList>
#include <optional>

class ChargingStationDao
{
public:
    static QList<ChargingStation> findAll();
    static std::optional<ChargingStation> findById(int id);
    static bool insert(const ChargingStation& station);
    static bool update(const ChargingStation& station);
    static bool deleteById(int id);
};

#endif // CHARGINGSTATIONDAO_H
