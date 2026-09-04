#include "mainwindow.h"
#include "serverservice.h"

#include "salespage.h"
#include "pilestatuspage.h"
#include "pilemanagepage.h"
#include "stationmanagepage.h"
#include "usermanagepage.h"
#include "orderspage.h"

#include <QTabWidget>
#include <QLabel>
#include <QStatusBar>
#include <QToolBar>

MainWindow::MainWindow(ServerService *service, QWidget *parent)
    : QMainWindow(parent)
    , m_service(service)
{
    setWindowTitle("充电桩应用管理平台 - 服务器端");
    resize(1100, 720);

    buildToolBar();
    buildPages();

    connect(m_service, &ServerService::listenStateChanged,
            this, &MainWindow::onListenStateChanged);
    connect(m_service, &ServerService::logMessage,
            this, &MainWindow::onLogMessage);
}

void MainWindow::buildToolBar()
{
    m_stateLabel = new QLabel("服务器状态：启动中...");
    m_logLabel = new QLabel("就绪");

    QToolBar *toolBar = addToolBar("状态");
    toolBar->setMovable(false);
    toolBar->addWidget(m_stateLabel);
    toolBar->addSeparator();
    toolBar->addWidget(m_logLabel);
}

void MainWindow::buildPages()
{
    QTabWidget *tabs = new QTabWidget(this);
    setCentralWidget(tabs);

    tabs->addTab(new SalesPage(m_service, tabs), "销售业绩");
    tabs->addTab(new PileStatusPage(m_service, tabs), "电桩状态");
    tabs->addTab(new PileManagePage(m_service, tabs), "充电桩管理");
    tabs->addTab(new StationManagePage(m_service, tabs), "充电站管理");
    tabs->addTab(new UserManagePage(m_service, tabs), "用户管理");
    tabs->addTab(new OrdersPage(m_service, tabs), "订单记录");
}

void MainWindow::onListenStateChanged(bool listening, quint16 port, const QString &address)
{
    if (listening) {
        m_stateLabel->setText(QString("服务器状态：运行中  端口 %1  地址 %2").arg(port).arg(address));
    } else {
        m_stateLabel->setText("服务器状态：已停止");
    }
}

void MainWindow::onLogMessage(const QString &message)
{
    m_logLabel->setText("日志：" + message);
}