#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class ServerService;
class QLabel;

// 服务器端主界面：顶部状态栏 + 各管理功能页签
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(ServerService *service, QWidget *parent = nullptr);

private slots:
    void onListenStateChanged(bool listening, quint16 port, const QString &address);
    void onLogMessage(const QString &message);

private:
    void buildToolBar();
    void buildPages();

    ServerService *m_service;
    QLabel        *m_stateLabel;
    QLabel        *m_logLabel;
};

#endif // MAINWINDOW_H