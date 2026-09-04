#include "chargepage.h"
#include "tcpclient.h"
#include "mainwindow.h"

#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QJsonArray>
#include <QDateTime>
#include <QPainter>

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

ChargePage::ChargePage(TcpClient *client, MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent)
    , m_client(client)
    , m_mainWindow(mainWindow)
{
    // ---- 顶部栏 ----
    QPushButton *backBtn = new QPushButton("返回");
    m_titleLabel = new QLabel("充电");
    QFont tf = m_titleLabel->font();
    tf.setPointSize(16);
    tf.setBold(true);
    m_titleLabel->setFont(tf);

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(backBtn);
    topLayout->addSpacing(8);
    topLayout->addWidget(m_titleLabel);
    topLayout->addStretch();

    m_pileInfoLabel = new QLabel;
    m_priceLabel = new QLabel;
    m_priceLabel->setStyleSheet("color: gray;");

    // ---- 充电中视图 ----
    m_chargingWidget = new QWidget;

    m_voltageLabel = new QLabel("-- V");
    m_currentLabel = new QLabel("-- A");
    m_powerLabel = new QLabel("-- kW");
    m_socLabel = new QLabel("0%");
    m_kwhLabel = new QLabel("0.00 kWh");
    m_remainLabel = new QLabel("--");

    m_socBar = new QProgressBar;
    m_socBar->setRange(0, 100);
    m_socBar->setTextVisible(false);

    m_powerSeries = new QLineSeries;
    m_powerChart = new QChart;
    m_powerChart->addSeries(m_powerSeries);
    m_powerChart->setTitle("功率曲线 (kW)");
    m_powerChart->legend()->hide();

    m_axisX = new QValueAxis;
    m_axisX->setRange(0, 60);
    m_axisX->setLabelFormat("%d");
    m_axisY = new QValueAxis;
    m_axisY->setRange(0, 20);
    m_axisY->setLabelFormat("%.1f");
    m_powerChart->addAxis(m_axisX, Qt::AlignBottom);
    m_powerChart->addAxis(m_axisY, Qt::AlignLeft);
    m_powerSeries->attachAxis(m_axisX);
    m_powerSeries->attachAxis(m_axisY);

    m_chartView = new QChartView(m_powerChart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(160);

    m_stopButton = new QPushButton("停止充电并结算");
    m_stopButton->setMinimumHeight(40);
    m_stopButton->setStyleSheet("background:#e8554e; color:white;");

    QGridLayout *dataGrid = new QGridLayout;
    dataGrid->addWidget(new QLabel("电压"), 0, 0);
    dataGrid->addWidget(m_voltageLabel, 1, 0);
    dataGrid->addWidget(new QLabel("电流"), 0, 1);
    dataGrid->addWidget(m_currentLabel, 1, 1);
    dataGrid->addWidget(new QLabel("功率"), 0, 2);
    dataGrid->addWidget(m_powerLabel, 1, 2);
    dataGrid->addWidget(new QLabel("已充电量"), 2, 0);
    dataGrid->addWidget(m_kwhLabel, 3, 0);
    dataGrid->addWidget(new QLabel("剩余时间"), 2, 1);
    dataGrid->addWidget(m_remainLabel, 3, 1);
    dataGrid->addWidget(new QLabel("SOC"), 2, 2);
    dataGrid->addWidget(m_socLabel, 3, 2);

    QVBoxLayout *chargingLayout = new QVBoxLayout(m_chargingWidget);
    chargingLayout->addLayout(dataGrid);
    chargingLayout->addWidget(new QLabel("电池电量"));
    chargingLayout->addWidget(m_socBar);
    chargingLayout->addWidget(m_chartView);
    chargingLayout->addWidget(m_stopButton);

    // ---- 结算视图 ----
    m_settledWidget = new QWidget;
    QLabel *settledTitle = new QLabel("充电结算");
    QFont sf = settledTitle->font();
    sf.setPointSize(16);
    sf.setBold(true);
    settledTitle->setFont(sf);
    settledTitle->setAlignment(Qt::AlignCenter);

    m_settledFeeLabel = new QLabel;
    m_settledKwhLabel = new QLabel;
    m_settledBalanceLabel = new QLabel;
    for (QLabel *l : { m_settledFeeLabel, m_settledKwhLabel, m_settledBalanceLabel }) {
        QFont f = l->font();
        f.setPointSize(14);
        l->setFont(f);
        l->setAlignment(Qt::AlignCenter);
    }

    m_finishButton = new QPushButton("完成");
    m_finishButton->setMinimumHeight(40);

    QVBoxLayout *settledLayout = new QVBoxLayout(m_settledWidget);
    settledLayout->addStretch();
    settledLayout->addWidget(settledTitle);
    settledLayout->addSpacing(20);
    settledLayout->addWidget(m_settledKwhLabel);
    settledLayout->addWidget(m_settledFeeLabel);
    settledLayout->addWidget(m_settledBalanceLabel);
    settledLayout->addSpacing(20);
    settledLayout->addWidget(m_finishButton);
    settledLayout->addStretch();

    // ---- 内容栈 ----
    m_contentStack = new QStackedWidget;
    m_contentStack->addWidget(m_chargingWidget);
    m_contentStack->addWidget(m_settledWidget);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->addLayout(topLayout);
    layout->addWidget(m_pileInfoLabel);
    layout->addWidget(m_priceLabel);
    layout->addWidget(m_contentStack, 1);

    connect(backBtn, &QPushButton::clicked, this, &ChargePage::goBack);
    connect(m_stopButton, &QPushButton::clicked, this, &ChargePage::onStopClicked);
    connect(m_finishButton, &QPushButton::clicked, this, &ChargePage::goBack);
    connect(m_client, &TcpClient::messageReceived, this, &ChargePage::onMessageReceived);
}

void ChargePage::prepare()
{
    m_pile = m_mainWindow->currentPile();
    m_orderId = 0;
    m_elapsedSec = 0;
    m_pricePerKwh = m_mainWindow->currentStation().pricePerKwh;

    m_pileInfoLabel->setText(QString("电桩 %1（%2，%3 kW）")
                                 .arg(m_pile.code)
                                 .arg(m_pile.type == "fast" ? "快充" : "慢充")
                                 .arg(m_pile.powerKw));
    m_priceLabel->setText(QString("单价 %1 元/度").arg(m_pricePerKwh));

    m_titleLabel->setText("充电");
    showChargingView();

    // 进入充电页时自动检查是否存在未完成订单
    QJsonObject payload;
    payload["user_id"] = m_client->currentUser().id;
    m_client->send(Protocol::MsgId::REQ_CHECK_PENDING, payload);
}

void ChargePage::showChargingView()
{
    m_contentStack->setCurrentWidget(m_chargingWidget);
}

void ChargePage::showSettledView(double kwh, double fee, double balance)
{
    m_settledKwhLabel->setText(QString("充电电量：%1 kWh").arg(kwh, 0, 'f', 2));
    m_settledFeeLabel->setText(QString("本次费用：%1 元").arg(fee, 0, 'f', 2));
    m_settledBalanceLabel->setText(QString("钱包余额：%1 元").arg(balance, 0, 'f', 2));
    m_titleLabel->setText("结算完成");
    m_contentStack->setCurrentWidget(m_settledWidget);
}

void ChargePage::updateRealtimeData(const QJsonObject &data)
{
    double voltage = data.value("voltage").toDouble();
    double current = data.value("current").toDouble();
    double power = data.value("power").toDouble();
    double soc = data.value("soc").toDouble();
    double kwh = data.value("kwh").toDouble();

    m_voltageLabel->setText(QString::number(voltage, 'f', 1) + " V");
    m_currentLabel->setText(QString::number(current, 'f', 1) + " A");
    m_powerLabel->setText(QString::number(power, 'f', 2) + " kW");
    m_socLabel->setText(QString::number(soc, 'f', 1) + "%");
    m_kwhLabel->setText(QString::number(kwh, 'f', 2) + " kWh");
    m_socBar->setValue(static_cast<int>(soc));

    // 预计剩余时间：按功率与电池容量(60kWh)估算
    if (power > 0.1) {
        double remainKwh = 60.0 * (100.0 - soc) / 100.0;
        int remainSec = static_cast<int>(remainKwh / power * 3600);
        int mm = remainSec / 60;
        int ss = remainSec % 60;
        m_remainLabel->setText(QString("%1:%2").arg(mm, 2, 10, QChar('0')).arg(ss, 2, 10, QChar('0')));
    }

    // 功率曲线追加采样点
    m_powerSeries->append(m_elapsedSec, power);
    if (m_elapsedSec > 60) {
        m_axisX->setRange(m_elapsedSec - 60, m_elapsedSec);
    } else {
        m_axisX->setRange(0, 60);
    }
    m_axisY->setRange(0, qMax(20.0, power * 1.2));
    ++m_elapsedSec;
}

void ChargePage::onMessageReceived(quint16 msgId, const QJsonObject &payload)
{
    switch (msgId) {
    case Protocol::MsgId::RES_CHECK_PENDING: {
        bool hasPending = payload.value("has_pending").toBool();
        if (hasPending) {
            QJsonObject order = payload.value("order").toObject();
            m_orderId = order.value("id").toInt();
            m_titleLabel->setText("继续未完成订单");
            QMessageBox::information(this, "提示", "您有未完成的充电订单，请先结算");
            showChargingView();
        } else {
            // 无未完成订单，启动新充电
            QJsonObject req;
            req["user_id"] = m_client->currentUser().id;
            req["pile_id"] = m_pile.id;
            m_client->send(Protocol::MsgId::REQ_START_CHARGE, req);
        }
        break;
    }
    case Protocol::MsgId::RES_START_CHARGE: {
        int code = payload.value("code").toInt();
        if (code == Protocol::ErrorCode::OK) {
            QJsonObject order = payload.value("order").toObject();
            m_orderId = order.value("id").toInt();
            m_pricePerKwh = payload.value("price_per_kwh").toDouble();
            m_titleLabel->setText("充电中");
            showChargingView();
        } else {
            QMessageBox::warning(this, "启动失败", "无法启动充电，错误码: " + QString::number(code));
            goBack();
        }
        break;
    }
    case Protocol::MsgId::RES_CHARGE_DATA: {
        updateRealtimeData(payload);
        break;
    }
    case Protocol::MsgId::RES_STOP_CHARGE: {
        int code = payload.value("code").toInt();
        if (code == Protocol::ErrorCode::OK) {
            QJsonObject order = payload.value("order").toObject();
            double balance = payload.value("balance").toDouble();
            showSettledView(order.value("kwh").toDouble(), order.value("fee").toDouble(), balance);
        }
        break;
    }
    default:
        break;
    }
}

void ChargePage::onStopClicked()
{
    if (m_orderId == 0) {
        return;
    }
    QJsonObject payload;
    payload["order_id"] = m_orderId;
    m_client->send(Protocol::MsgId::REQ_STOP_CHARGE, payload);
}

void ChargePage::goBack()
{
    m_mainWindow->switchTo(MainWindow::PageStationList);
}