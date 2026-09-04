#ifndef STATIONLISTPAGE_H
#define STATIONLISTPAGE_H

#include <QWidget>
#include <QList>
#include <QJsonObject>

class QComboBox;
class QLineEdit;
class QPushButton;
class QListWidget;
class QLabel;
class TcpClient;
class MainWindow;
class QNetworkAccessManager;

// 充电站列表页：定位（模拟 GPS + 地址地理编码）→ 展示附近充电站
class StationListPage : public QWidget
{
    Q_OBJECT
public:
    explicit StationListPage(TcpClient *client, MainWindow *mainWindow, QWidget *parent = nullptr);

private slots:
    void onLocate();
    void onMessageReceived(quint16 msgId, const QJsonObject &payload);
    void onGeocodeFinished();

private:
    void requestStationList();
    void rebuildList();
    void goToProfile();

    TcpClient *m_client;
    MainWindow *m_mainWindow;

    QComboBox   *m_areaCombo;
    QLineEdit   *m_addressEdit;
    QPushButton *m_locateButton;
    QListWidget *m_stationList;
    QLabel      *m_locationLabel;

    QNetworkAccessManager *m_network;

    double m_userLng = 120.21;  // 当前模拟 GPS 经度
    double m_userLat = 30.20;   // 当前模拟 GPS 纬度

    QList<QJsonObject> m_stations;
};

#endif // STATIONLISTPAGE_H