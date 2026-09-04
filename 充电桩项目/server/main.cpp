#include <QApplication>
#include <QThread>
#include <QDir>
#include <QMetaObject>
#include <QDebug>

#include "serverservice.h"
#include "adminloginwidget.h"
#include "mainwindow.h"

// 注册跨线程信号槽所需的自定义类型
static void registerMetaTypes()
{
    qRegisterMetaType<UserInfo>("UserInfo");
    qRegisterMetaType<StationInfo>("StationInfo");
    qRegisterMetaType<PileInfo>("PileInfo");
    qRegisterMetaType<OrderInfo>("OrderInfo");
    qRegisterMetaType<QList<UserInfo>>("QList<UserInfo>");
    qRegisterMetaType<QList<StationInfo>>("QList<StationInfo>");
    qRegisterMetaType<QList<PileInfo>>("QList<PileInfo>");
    qRegisterMetaType<QList<OrderInfo>>("QList<OrderInfo>");
    qRegisterMetaType<QPair<QString, double>>("QPair<QString,double>");
    qRegisterMetaType<QList<QPair<QString, double>>>("QList<QPair<QString,double>>");
    qRegisterMetaType<QMap<QString, int>>("QMap<QString,int>");
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("充电桩管理平台 - 服务器端");

    registerMetaTypes();

    // 业务线程：承载 ServerService（网络 + 业务 + 数据库），与 UI 主线程分离
    QThread workerThread;
    workerThread.setObjectName("ServerBusinessThread");

    ServerService *service = new ServerService();
    service->moveToThread(&workerThread);

    QObject::connect(&workerThread, &QThread::finished, service, &QObject::deleteLater);
    workerThread.start();

    // 数据库文件放在可执行文件同目录
    QString dbPath = QCoreApplication::applicationDirPath() + "/charging_platform.db";
    QMetaObject::invokeMethod(service, "initializeDatabase", Qt::QueuedConnection,
                              Q_ARG(QString, dbPath));

    // 管理员登录对话框
    AdminLoginWidget loginWidget(service);
    if (loginWidget.exec() != QDialog::Accepted) {
        workerThread.quit();
        workerThread.wait();
        return 0;
    }

    // 登录成功，进入主界面并启动服务器
    MainWindow mainWindow(service);
    mainWindow.show();

    const quint16 defaultPort = 9527;
    QMetaObject::invokeMethod(service, "startServer", Qt::QueuedConnection,
                              Q_ARG(quint16, defaultPort));

    int result = app.exec();

    // 退出前停止服务器并回收业务线程
    QMetaObject::invokeMethod(service, "stopServer", Qt::BlockingQueuedConnection);
    workerThread.quit();
    workerThread.wait();

    return result;
}