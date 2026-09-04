#ifndef WALLETRECHARGEDAO_H
#define WALLETRECHARGEDAO_H

#include "src/dto/walletrecharge.h"
#include <QList>
#include <optional>

class WalletRechargeDao
{
public:
    static QList<WalletRecharge> findAll();
    static QList<WalletRecharge> findByUserId(int userId);
    static bool insert(const WalletRecharge& recharge);
};

#endif // WALLETRECHARGEDAO_H
