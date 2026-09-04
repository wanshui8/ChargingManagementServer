#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

#include "datatypes.h"

class QStackedWidget;
class TcpClient;
class LoginPage;
class StationListPage;
class PileDetailPage;
class ChargePage;
class ProfilePage;
class MapView;

// ============================================================================
// 用户端主窗口：竖屏手机风格，通过 QStackedWidget 管理多个页面切换
// ============================================================================
class MainWindow : public QWidget
{
    Q_OBJECT
public:
    enum Page {
        PageLogin = 0,
        PageStationList,
        PagePileDetail,
        PageCharge,
        PageProfile,
        PageMap
    };

    explicit MainWindow(TcpClient *client, QWidget *parent = nullptr);

    TcpClient *client() const { return m_client; }

    void switchTo(Page page);

    // 页面间共享的上下文数据
    StationInfo currentStation() const { return m_currentStation; }
    void setCurrentStation(const StationInfo &s) { m_currentStation = s; }

    PileInfo currentPile() const { return m_currentPile; }
    void setCurrentPile(const PileInfo &p) { m_currentPile = p; }

    int currentOrderId() const { return m_currentOrderId; }
    void setCurrentOrderId(int id) { m_currentOrderId = id; }

    MapView *mapView() const { return m_mapView; }

    // 页面跳转入口（封装：设置共享数据 + 刷新目标页 + 切换）
    void showStationDetail(const StationInfo &station);
    void showChargePage(const PileInfo &pile);
    void showMap(double fromLng, double fromLat, double toLng, double toLat);

private:
    void buildPages();

    TcpClient      *m_client;
    QStackedWidget *m_stack;

    LoginPage       *m_loginPage;
    StationListPage *m_stationListPage;
    PileDetailPage  *m_pileDetailPage;
    ChargePage      *m_chargePage;
    ProfilePage     *m_profilePage;
    MapView         *m_mapView;

    StationInfo m_currentStation;
    PileInfo    m_currentPile;
    int         m_currentOrderId = 0;
};

#endif // MAINWINDOW_H