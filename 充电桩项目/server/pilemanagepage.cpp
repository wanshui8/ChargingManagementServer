#include "pilemanagepage.h"
#include "serverservice.h"

#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

static QString pileStatusCn(const QString &s)
{
    if (s == "idle") return "空闲";
    if (s == "charging") return "充电中";
    if (s == "fault") return "故障";
    if (s == "offline") return "离线";
    return s;
}

PileManagePage::PileManagePage(ServerService *service, QWidget *parent)
    : QWidget(parent)
    , m_service(service)
{
    QPushButton *refreshBtn = new QPushButton("刷新");
    m_restartButton = new QPushButton("远程重启");
    m_restartButton->setEnabled(false);

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(refreshBtn);
    topLayout->addWidget(m_restartButton);
    topLayout->addStretch();

    m_table = new QTableWidget(this);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({ "编号", "所属电站", "类型", "功率(kW)", "状态", "累计次数", "累计时长(分钟)" });
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_table);

    connect(m_service, &ServerService::pilesReady,
            this, &PileManagePage::onPilesReady);
    connect(m_service, &ServerService::stationsReady,
            this, &PileManagePage::onStationsReady);
    connect(refreshBtn, &QPushButton::clicked, this, &PileManagePage::onRefresh);
    connect(m_restartButton, &QPushButton::clicked, this, &PileManagePage::onRestart);
    connect(m_table, &QTableWidget::itemSelectionChanged,
            this, [this]() { m_restartButton->setEnabled(m_table->currentRow() >= 0); });

    onRefresh();
}

void PileManagePage::onRefresh()
{
    m_service->asyncLoadStations();
    m_service->asyncLoadPiles();
}

void PileManagePage::onStationsReady(const QList<StationInfo> &stations)
{
    m_stationNames.clear();
    for (const StationInfo &s : stations) {
        m_stationNames.insert(s.id, s.name);
    }
    rebuildTable();
}

void PileManagePage::onPilesReady(const QList<PileInfo> &piles)
{
    m_piles = piles;
    rebuildTable();
}

void PileManagePage::rebuildTable()
{
    m_table->setRowCount(m_piles.size());
    for (int i = 0; i < m_piles.size(); ++i) {
        const PileInfo &p = m_piles.at(i);
        m_table->setItem(i, 0, new QTableWidgetItem(p.code));
        m_table->setItem(i, 1, new QTableWidgetItem(m_stationNames.value(p.stationId, QString::number(p.stationId))));
        m_table->setItem(i, 2, new QTableWidgetItem(p.type == "fast" ? "快充" : "慢充"));
        m_table->setItem(i, 3, new QTableWidgetItem(QString::number(p.powerKw)));
        m_table->setItem(i, 4, new QTableWidgetItem(pileStatusCn(p.status)));
        m_table->setItem(i, 5, new QTableWidgetItem(QString::number(p.chargeCount)));
        m_table->setItem(i, 6, new QTableWidgetItem(QString::number(p.chargeMinutes)));

        // 存储桩 ID 供重启使用
        m_table->item(i, 0)->setData(Qt::UserRole, p.id);
    }
}

void PileManagePage::onRestart()
{
    int row = m_table->currentRow();
    if (row < 0) {
        return;
    }
    int pileId = m_table->item(row, 0)->data(Qt::UserRole).toInt();
    m_service->asyncRestartPile(pileId);
    QMessageBox::information(this, "提示", "已发送远程重启指令（模拟）");
}