#ifndef ORDERSPAGE_H
#define ORDERSPAGE_H

#include <QWidget>
#include <QList>

#include "database.h"

class ServerService;
class QTableWidget;

// 订单记录页：展示全平台充电订单
class OrdersPage : public QWidget
{
    Q_OBJECT
public:
    explicit OrdersPage(ServerService *service, QWidget *parent = nullptr);

private slots:
    void onOrdersReady(const QList<OrderInfo> &orders);
    void onRefresh();

private:
    ServerService *m_service;
    QTableWidget   *m_table;
};

#endif // ORDERSPAGE_H