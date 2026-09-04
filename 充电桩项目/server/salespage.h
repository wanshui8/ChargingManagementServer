#ifndef SALESPAGE_H
#define SALESPAGE_H

#include <QWidget>
#include <QList>
#include <QPair>

class ServerService;
class QLabel;
class QChartView;
class QPushButton;

// 销售业绩页：营收趋势折线图 + 三大核心指标
class SalesPage : public QWidget
{
    Q_OBJECT
public:
    explicit SalesPage(ServerService *service, QWidget *parent = nullptr);

private slots:
    void onRevenueReady(const QList<QPair<QString, double>> &trend,
                        double today, double month, double total);
    void onRefresh();
    void show7Days();
    void show30Days();

private:
    void updateChart();

    ServerService *m_service;
    QLabel        *m_todayLabel;
    QLabel        *m_monthLabel;
    QLabel        *m_totalLabel;
    QChartView    *m_chartView;
    QPushButton   *m_btn7;
    QPushButton   *m_btn30;

    QList<QPair<QString, double>> m_trend30;
    int m_days = 30;
};

#endif // SALESPAGE_H