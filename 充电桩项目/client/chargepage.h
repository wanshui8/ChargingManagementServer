#ifndef CHARGEPAGE_H
#define CHARGEPAGE_H

#include <QWidget>
#include <QJsonObject>

#include "datatypes.h"

class QLabel;
class QPushButton;
class QProgressBar;
class QStackedWidget;
class QChartView;
class QLineSeries;
class QChart;
class QValueAxis;
class TcpClient;
class MainWindow;

// 充电页：检查未完成订单 → 启动充电 → 实时采样看板 → 停止结算
class ChargePage : public QWidget
{
    Q_OBJECT
public:
    explicit ChargePage(TcpClient *client, MainWindow *mainWindow, QWidget *parent = nullptr);

    // 进入本页时调用：显示电桩信息并检查未完成订单
    void prepare();

private slots:
    void onMessageReceived(quint16 msgId, const QJsonObject &payload);
    void onStopClicked();
    void goBack();

private:
    void showChargingView();
    void showSettledView(double kwh, double fee, double balance);
    void updateRealtimeData(const QJsonObject &data);

    TcpClient *m_client;
    MainWindow *m_mainWindow;

    QLabel      *m_titleLabel;
    QLabel      *m_pileInfoLabel;
    QLabel      *m_priceLabel;

    // 充电中视图
    QWidget     *m_chargingWidget;
    QLabel      *m_voltageLabel;
    QLabel      *m_currentLabel;
    QLabel      *m_powerLabel;
    QLabel      *m_socLabel;
    QLabel      *m_kwhLabel;
    QLabel      *m_remainLabel;
    QProgressBar *m_socBar;
    QChartView  *m_chartView;
    QLineSeries *m_powerSeries;
    QChart      *m_powerChart;
    QValueAxis  *m_axisX;
    QValueAxis  *m_axisY;
    QPushButton *m_stopButton;

    // 结算视图
    QWidget     *m_settledWidget;
    QLabel      *m_settledFeeLabel;
    QLabel      *m_settledKwhLabel;
    QLabel      *m_settledBalanceLabel;
    QPushButton *m_finishButton;

    QStackedWidget *m_contentStack;

    int     m_orderId = 0;
    double  m_pricePerKwh = 0.0;
    PileInfo m_pile;
    int     m_elapsedSec = 0;
};

#endif // CHARGEPAGE_H