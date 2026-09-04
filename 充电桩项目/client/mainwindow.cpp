#include "mainwindow.h"
#include "tcpclient.h"

#include "loginpage.h"
#include "stationlistpage.h"
#include "piledetailpage.h"
#include "chargepage.h"
#include "profilepage.h"
#include "mapview.h"

#include <QStackedWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(TcpClient *client, QWidget *parent)
    : QWidget(parent)
    , m_client(client)
{
    setWindowTitle("电动充电用户端");
    setFixedSize(420, 760);   // 竖屏手机风格

    m_stack = new QStackedWidget(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_stack);

    buildPages();
    switchTo(PageLogin);
}

void MainWindow::buildPages()
{
    m_loginPage = new LoginPage(m_client, this);
    m_stationListPage = new StationListPage(m_client, this);
    m_pileDetailPage = new PileDetailPage(m_client, this);
    m_chargePage = new ChargePage(m_client, this);
    m_profilePage = new ProfilePage(m_client, this);
    m_mapView = new MapView(this);

    m_stack->addWidget(m_loginPage);        // PageLogin
    m_stack->addWidget(m_stationListPage);  // PageStationList
    m_stack->addWidget(m_pileDetailPage);   // PagePileDetail
    m_stack->addWidget(m_chargePage);       // PageCharge
    m_stack->addWidget(m_profilePage);      // PageProfile
    m_stack->addWidget(m_mapView);          // PageMap

    // 地图页返回 → 回到充电站列表
    connect(m_mapView, &MapView::backRequested,
            this, [this]() { switchTo(PageStationList); });
}

void MainWindow::switchTo(Page page)
{
    m_stack->setCurrentIndex(static_cast<int>(page));
}

void MainWindow::showStationDetail(const StationInfo &station)
{
    setCurrentStation(station);
    m_pileDetailPage->refresh();
    switchTo(PagePileDetail);
}

void MainWindow::showChargePage(const PileInfo &pile)
{
    setCurrentPile(pile);
    m_chargePage->prepare();
    switchTo(PageCharge);
}

void MainWindow::showMap(double fromLng, double fromLat, double toLng, double toLat)
{
    m_mapView->navigate(fromLng, fromLat, toLng, toLat);
    switchTo(PageMap);
}