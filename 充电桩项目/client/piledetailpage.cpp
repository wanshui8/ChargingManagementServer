#include "piledetailpage.h"
#include "tcpclient.h"
#include "mainwindow.h"

#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QMessageBox>

static QString pileStatusCn(const QString &s)
{
    if (s == "idle") return "空闲";
    if (s == "charging") return "充电中";
    if (s == "fault") return "故障";
    if (s == "offline") return "离线";
    return s;
}

PileDetailPage::PileDetailPage(TcpClient *client, MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent)
    , m_client(client)
    , m_mainWindow(mainWindow)
{
    QPushButton *backBtn = new QPushButton("返回");
    m_titleLabel = new QLabel("充电站详情");
    QFont f = m_titleLabel->font();
    f.setPointSize(16);
    f.setBold(true);
    m_titleLabel->setFont(f);

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(backBtn);
    topLayout->addSpacing(8);
    topLayout->addWidget(m_titleLabel);
    topLayout->addStretch();

    QLabel *tipLabel = new QLabel("点击【空闲】电桩开始充电");
    tipLabel->setStyleSheet("color: gray;");

    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({ "编号", "类型", "功率(kW)", "状态" });
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->verticalHeader()->setVisible(false);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->addLayout(topLayout);
    layout->addWidget(tipLabel);
    layout->addWidget(m_table, 1);

    connect(backBtn, &QPushButton::clicked, this, &PileDetailPage::goBack);
    connect(m_table, &QTableWidget::cellClicked, this, &PileDetailPage::onPileRowClicked);
    connect(m_client, &TcpClient::messageReceived, this, &PileDetailPage::onMessageReceived);
}

void PileDetailPage::refresh()
{
    StationInfo station = m_mainWindow->currentStation();
    m_titleLabel->setText(station.name);
    m_titleLabel->setToolTip(station.address);

    QJsonObject payload;
    payload["station_id"] = station.id;
    m_client->send(Protocol::MsgId::REQ_PILE_LIST, payload);
}

void PileDetailPage::onMessageReceived(quint16 msgId, const QJsonObject &payload)
{
    if (msgId != Protocol::MsgId::RES_PILE_LIST) {
        return;
    }

    QJsonArray arr = payload.value("piles").toArray();
    m_piles.clear();
    for (const QJsonValue &v : arr) {
        QJsonObject o = v.toObject();
        PileInfo p;
        p.id = o.value("id").toInt();
        p.stationId = o.value("station_id").toInt();
        p.code = o.value("code").toString();
        p.type = o.value("type").toString();
        p.powerKw = o.value("power_kw").toDouble();
        p.status = o.value("status").toString();
        p.chargeCount = o.value("charge_count").toInt();
        p.chargeMinutes = o.value("charge_minutes").toInt();
        m_piles.append(p);
    }
    rebuildTable();
}

void PileDetailPage::rebuildTable()
{
    m_table->setRowCount(m_piles.size());
    for (int i = 0; i < m_piles.size(); ++i) {
        const PileInfo &p = m_piles.at(i);
        m_table->setItem(i, 0, new QTableWidgetItem(p.code));
        m_table->setItem(i, 1, new QTableWidgetItem(p.type == "fast" ? "快充" : "慢充"));
        m_table->setItem(i, 2, new QTableWidgetItem(QString::number(p.powerKw)));
        m_table->setItem(i, 3, new QTableWidgetItem(pileStatusCn(p.status)));
        m_table->item(i, 3)->setForeground(p.status == "idle" ? QBrush(QColor("#2f7eea")) : QBrush(Qt::black));
    }
}

void PileDetailPage::onPileRowClicked(int row, int column)
{
    Q_UNUSED(column);
    if (row < 0 || row >= m_piles.size()) {
        return;
    }
    const PileInfo &pile = m_piles.at(row);
    if (pile.status != "idle") {
        QMessageBox::information(this, "提示", "该电桩当前" + pileStatusCn(pile.status) + "，请选择空闲电桩");
        return;
    }
    m_mainWindow->showChargePage(pile);
}

void PileDetailPage::goBack()
{
    m_mainWindow->switchTo(MainWindow::PageStationList);
}