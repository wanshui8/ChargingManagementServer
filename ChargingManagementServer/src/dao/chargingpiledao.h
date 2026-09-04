#ifndef CHARGINGPIDEDAO_H
#define CHARGINGPIDEDAO_H

#include "src/dto/chargingpile.h"
#include <QList>
#include <optional>

class ChargingPileDao
{
public:
    static QList<ChargingPile> findAll();
    static QList<ChargingPile> findByStationId(int stationId);
    static std::optional<ChargingPile> findById(int id);
    static bool insert(const ChargingPile& pile);
    static bool updateStatus(int id, int status);
    static bool deleteById(int id);
};

#endif // CHARGINGPIDEDAO_H
