#ifndef PILEMANAGEPAGE_H
#define PILEMANAGEPAGE_H

#include <QWidget>
#include <QList>
#include <QMap>

#include "database.h"

class ServerService;
class QTableWidget;
class QPushButton;

// 充电桩管理页：列表展示 + 远程重启
class PileManagePage : public QWidget
{
    Q_OBJECT
public:
    explicit PileManagePage(ServerService *service, QWidget *parent = nullptr);

private slots:
    void onPilesReady(const QList<PileInfo> &piles);
    void onStationsReady(const QList<StationInfo> &stations);
    void onRefresh();
    void onRestart();

private:
    void rebuildTable();

    ServerService *m_service;
    QTableWidget   *m_table;
    QPushButton    *m_restartButton;

    QList<PileInfo> m_piles;
    QMap<int, QString> m_stationNames;
};

#endif // PILEMANAGEPAGE_H