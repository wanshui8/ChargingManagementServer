#include "stationmanagepage.h"
#include "serverservice.h"

#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

StationManagePage::StationManagePage(ServerService *service, QWidget *parent)
    : QWidget(parent)
    , m_service(service)
{
    QPushButton *refreshBtn = new QPushButton("刷新");
    QPushButton *addBtn = new QPushButton("新增电站");

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(refreshBtn);
    topLayout->addWidget(addBtn);
    topLayout->addStretch();

    m_table = new QTableWidget(this);
    m_table->setColumnCount(9);
    m_table->setHorizontalHeaderLabels({ "站ID", "站名", "地址", "经度", "纬度", "价格(元/度)", "总桩数", "空闲数", "在线率" });
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_table);

    connect(m_service, &ServerService::stationsReady,
            this, &StationManagePage::onStationsReady);
    connect(m_service, &ServerService::pilesReady,
            this, &StationManagePage::onPilesReady);
    connect(refreshBtn, &QPushButton::clicked, this, &StationManagePage::onRefresh);
    connect(addBtn, &QPushButton::clicked, this, &StationManagePage::onAddStation);
    connect(m_table, &QTableWidget::cellDoubleClicked,
            this, &StationManagePage::onViewDetail);

    onRefresh();
}

void StationManagePage::onRefresh()
{
    m_service->asyncLoadStations();
    m_service->asyncLoadPiles();
}

void StationManagePage::onStationsReady(const QList<StationInfo> &stations)
{
    m_stations = stations;
    rebuildTable();
}

void StationManagePage::onPilesReady(const QList<PileInfo> &piles)
{
    m_piles = piles;
}

void StationManagePage::rebuildTable()
{
    m_table->setRowCount(m_stations.size());
    for (int i = 0; i < m_stations.size(); ++i) {
        const StationInfo &s = m_stations.at(i);
        int onlineRate = s.totalPiles > 0
                ? static_cast<int>(s.onlinePiles * 100.0 / s.totalPiles)
                : 0;

        m_table->setItem(i, 0, new QTableWidgetItem(QString::number(s.id)));
        m_table->setItem(i, 1, new QTableWidgetItem(s.name));
        m_table->setItem(i, 2, new QTableWidgetItem(s.address));
        m_table->setItem(i, 3, new QTableWidgetItem(QString::number(s.longitude, 'f', 6)));
        m_table->setItem(i, 4, new QTableWidgetItem(QString::number(s.latitude, 'f', 6)));
        m_table->setItem(i, 5, new QTableWidgetItem(QString::number(s.pricePerKwh, 'f', 2)));
        m_table->setItem(i, 6, new QTableWidgetItem(QString::number(s.totalPiles)));
        m_table->setItem(i, 7, new QTableWidgetItem(QString::number(s.idlePiles)));
        m_table->setItem(i, 8, new QTableWidgetItem(QString::number(onlineRate) + "%"));
    }
}

void StationManagePage::onAddStation()
{
    QDialog dialog(this);
    dialog.setWindowTitle("新增充电站");
    dialog.setFixedWidth(360);

    QLineEdit *nameEdit = new QLineEdit;
    QLineEdit *addrEdit = new QLineEdit;
    QDoubleSpinBox *lngSpin = new QDoubleSpinBox;
    lngSpin->setRange(-180.0, 180.0);
    lngSpin->setDecimals(6);
    lngSpin->setValue(120.20);
    QDoubleSpinBox *latSpin = new QDoubleSpinBox;
    latSpin->setRange(-90.0, 90.0);
    latSpin->setDecimals(6);
    latSpin->setValue(30.20);
    QDoubleSpinBox *priceSpin = new QDoubleSpinBox;
    priceSpin->setRange(0.1, 10.0);
    priceSpin->setDecimals(2);
    priceSpin->setValue(1.0);
    QSpinBox *pileCountSpin = new QSpinBox;
    pileCountSpin->setRange(1, 50);
    pileCountSpin->setValue(4);

    QFormLayout *form = new QFormLayout;
    form->addRow("站名：", nameEdit);
    form->addRow("地址：", addrEdit);
    form->addRow("经度：", lngSpin);
    form->addRow("纬度：", latSpin);
    form->addRow("价格(元/度)：", priceSpin);
    form->addRow("电桩数量：", pileCountSpin);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->addLayout(form);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    if (nameEdit->text().isEmpty() || addrEdit->text().isEmpty()) {
        QMessageBox::warning(this, "提示", "站名和地址不能为空");
        return;
    }

    m_service->asyncAddStation(nameEdit->text(), addrEdit->text(),
                               lngSpin->value(), latSpin->value(),
                               priceSpin->value(), pileCountSpin->value());
}

void StationManagePage::onViewDetail(int row, int column)
{
    Q_UNUSED(column);
    if (row < 0 || row >= m_stations.size()) {
        return;
    }
    int stationId = m_stations.at(row).id;

    // 弹出该站电桩详情
    QDialog dialog(this);
    dialog.setWindowTitle(m_stations.at(row).name + " - 电桩详情");
    dialog.resize(600, 400);

    QTableWidget *pileTable = new QTableWidget;
    pileTable->setColumnCount(5);
    pileTable->setHorizontalHeaderLabels({ "编号", "类型", "功率(kW)", "状态", "累计次数" });
    pileTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    pileTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QList<PileInfo> stationPiles;
    for (const PileInfo &p : m_piles) {
        if (p.stationId == stationId) {
            stationPiles.append(p);
        }
    }
    pileTable->setRowCount(stationPiles.size());
    for (int i = 0; i < stationPiles.size(); ++i) {
        const PileInfo &p = stationPiles.at(i);
        pileTable->setItem(i, 0, new QTableWidgetItem(p.code));
        pileTable->setItem(i, 1, new QTableWidgetItem(p.type == "fast" ? "快充" : "慢充"));
        pileTable->setItem(i, 2, new QTableWidgetItem(QString::number(p.powerKw)));
        pileTable->setItem(i, 3, new QTableWidgetItem(p.status));
        pileTable->setItem(i, 4, new QTableWidgetItem(QString::number(p.chargeCount)));
    }

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->addWidget(pileTable);
    dialog.exec();
}