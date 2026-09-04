#include <QApplication>
#include <QMetaType>

#include "tcpclient.h"
#include "mainwindow.h"

static void registerMetaTypes()
{
    qRegisterMetaType<UserInfo>("UserInfo");
    qRegisterMetaType<StationInfo>("StationInfo");
    qRegisterMetaType<PileInfo>("PileInfo");
    qRegisterMetaType<OrderInfo>("OrderInfo");
    qRegisterMetaType<QList<StationInfo>>("QList<StationInfo>");
    qRegisterMetaType<QList<PileInfo>>("QList<PileInfo>");
    qRegisterMetaType<QList<OrderInfo>>("QList<OrderInfo>");
}

int main(int argc, char *argv[])
{
    // 高 DPI 适配
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    app.setApplicationName("充电桩应用管理平台 - 用户端");

    registerMetaTypes();

    TcpClient *client = new TcpClient();

    MainWindow mainWindow(client);
    mainWindow.show();

    return app.exec();
}