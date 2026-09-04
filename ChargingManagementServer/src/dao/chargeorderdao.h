#ifndef CHARGEORDERDAO_H
#define CHARGEORDERDAO_H

#include "src/dto/chargeorder.h"
#include <QList>
#include <optional>

class ChargeOrderDao
{
public:
    static QList<ChargeOrder> findAll();
    static QList<ChargeOrder> findByUserId(int userId);
    static std::optional<ChargeOrder> findById(int id);
    static bool insert(const ChargeOrder& order);
    static bool updateStatus(int id, int status);
};

#endif // CHARGEORDERDAO_H
