#include "orderspage.h"
#include "serverservice.h"

#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

OrdersPage::OrdersPage(ServerService *service, QWidget *parent)
    : QWidget(parent)
    , m_service(service)
{
    QPushButton *refreshBtn = new QPushButton("刷新");
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(refreshBtn);
    topLayout->addStretch();

    m_table = new QTableWidget(this);
    m_table->setColumnCount(9);
    m_table->setHorizontalHeaderLabels({ "订单ID", "用户ID", "电桩ID", "站ID", "状态", "开始时间", "结束时间", "电量(kWh)", "费用(元)" });
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_table);

    connect(m_service, &ServerService::ordersReady,
            this, &OrdersPage::onOrdersReady);
    connect(refreshBtn, &QPushButton::clicked, this, &OrdersPage::onRefresh);

    onRefresh();
}

void OrdersPage::onRefresh()
{
    m_service->asyncLoadOrders();
}

void OrdersPage::onOrdersReady(const QList<OrderInfo> &orders)
{
    m_table->setRowCount(orders.size());
    for (int i = 0; i < orders.size(); ++i) {
        const OrderInfo &o = orders.at(i);
        QString statusCn = (o.status == "charging") ? "充电中" : "已结算";

        m_table->setItem(i, 0, new QTableWidgetItem(QString::number(o.id)));
        m_table->setItem(i, 1, new QTableWidgetItem(QString::number(o.userId)));
        m_table->setItem(i, 2, new QTableWidgetItem(QString::number(o.pileId)));
        m_table->setItem(i, 3, new QTableWidgetItem(QString::number(o.stationId)));
        m_table->setItem(i, 4, new QTableWidgetItem(statusCn));
        m_table->setItem(i, 5, new QTableWidgetItem(o.startTime));
        m_table->setItem(i, 6, new QTableWidgetItem(o.endTime));
        m_table->setItem(i, 7, new QTableWidgetItem(QString::number(o.kwh, 'f', 2)));
        m_table->setItem(i, 8, new QTableWidgetItem(QString::number(o.fee, 'f', 2)));
    }
}