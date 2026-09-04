#include "pilestatuspage.h"
#include "serverservice.h"

#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

PileStatusPage::PileStatusPage(ServerService *service, QWidget *parent)
    : QWidget(parent)
    , m_service(service)
{
    m_totalLabel = new QLabel("全平台电桩总数：0");
    QFont f = m_totalLabel->font();
    f.setPointSize(12);
    m_totalLabel->setFont(f);

    QPushButton *refreshBtn = new QPushButton("刷新");
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(m_totalLabel);
    topLayout->addStretch();
    topLayout->addWidget(refreshBtn);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({ "状态", "数量", "占比", "说明" });
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_table);

    connect(m_service, &ServerService::pileStatusReady,
            this, &PileStatusPage::onPileStatusReady);
    connect(refreshBtn, &QPushButton::clicked, this, &PileStatusPage::onRefresh);

    onRefresh();
}

void PileStatusPage::onRefresh()
{
    m_service->asyncLoadPileStatus();
}

void PileStatusPage::onPileStatusReady(const QMap<QString, int> &summary)
{
    int total = 0;
    for (int v : summary.values()) {
        total += v;
    }
    m_totalLabel->setText(QString("全平台电桩总数：%1").arg(total));

    struct Row { QString status; QString cn; QString desc; };
    QList<Row> rows = {
        { "idle",     "空闲",   "可立即使用" },
        { "charging", "充电中", "正在为车辆充电" },
        { "fault",    "故障",   "需人工检修" },
        { "offline",  "离线",   "通信异常" },
    };

    m_table->setRowCount(rows.size());
    for (int i = 0; i < rows.size(); ++i) {
        const Row &r = rows.at(i);
        int count = summary.value(r.status, 0);
        double percent = total > 0 ? (count * 100.0 / total) : 0.0;

        m_table->setItem(i, 0, new QTableWidgetItem(r.cn));
        m_table->setItem(i, 1, new QTableWidgetItem(QString::number(count)));
        m_table->setItem(i, 2, new QTableWidgetItem(QString::number(percent, 'f', 1) + "%"));
        m_table->setItem(i, 3, new QTableWidgetItem(r.desc));
    }
}