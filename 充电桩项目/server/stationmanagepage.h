#ifndef STATIONMANAGEPAGE_H
#define STATIONMANAGEPAGE_H

#include <QWidget>
#include <QList>

#include "database.h"

class ServerService;
class QTableWidget;

// 充电站管理页：列表展示 + 新增电站 + 查看站内电桩
class StationManagePage : public QWidget
{
    Q_OBJECT
public:
    explicit StationManagePage(ServerService *service, QWidget *parent = nullptr);

private slots:
    void onStationsReady(const QList<StationInfo> &stations);
    void onPilesReady(const QList<PileInfo> &piles);
    void onRefresh();
    void onAddStation();
    void onViewDetail(int row, int column);

private:
    void rebuildTable();

    ServerService *m_service;
    QTableWidget   *m_table;

    QList<StationInfo> m_stations;
    QList<PileInfo>    m_piles;
};

#endif // STATIONMANAGEPAGE_H