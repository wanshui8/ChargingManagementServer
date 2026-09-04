#ifndef PILESTATUSPAGE_H
#define PILESTATUSPAGE_H

#include <QWidget>
#include <QMap>

class ServerService;
class QTableWidget;
class QLabel;

// 电桩状态页：展示全平台电桩状态分布与占比
class PileStatusPage : public QWidget
{
    Q_OBJECT
public:
    explicit PileStatusPage(ServerService *service, QWidget *parent = nullptr);

private slots:
    void onPileStatusReady(const QMap<QString, int> &summary);
    void onRefresh();

private:
    ServerService *m_service;
    QTableWidget   *m_table;
    QLabel         *m_totalLabel;
};

#endif // PILESTATUSPAGE_H