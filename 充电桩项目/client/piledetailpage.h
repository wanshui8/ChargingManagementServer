#ifndef PILEDETAILPAGE_H
#define PILEDETAILPAGE_H

#include <QWidget>
#include <QList>
#include <QJsonObject>

#include "datatypes.h"

class QTableWidget;
class QLabel;
class TcpClient;
class MainWindow;

// 电桩详情页：展示某充电站下的所有电桩，点击空闲桩进入充电
class PileDetailPage : public QWidget
{
    Q_OBJECT
public:
    explicit PileDetailPage(TcpClient *client, MainWindow *mainWindow, QWidget *parent = nullptr);

    // 进入本页时调用：读取当前电站并请求电桩列表
    void refresh();

private slots:
    void onMessageReceived(quint16 msgId, const QJsonObject &payload);
    void onPileRowClicked(int row, int column);
    void goBack();

private:
    void rebuildTable();

    TcpClient *m_client;
    MainWindow *m_mainWindow;

    QLabel      *m_titleLabel;
    QTableWidget *m_table;

    QList<PileInfo> m_piles;
};

#endif // PILEDETAILPAGE_H