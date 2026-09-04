#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QWidget>

class QWebEngineView;
class QLabel;

// 地图导航页：用 QWebEngineView 加载腾讯地图 GL JS，规划驾车路线
class MapView : public QWidget
{
    Q_OBJECT
public:
    explicit MapView(QWidget *parent = nullptr);

    // 设置起点终点并加载地图路线规划
    void navigate(double fromLng, double fromLat, double toLng, double toLat);

signals:
    void backRequested();

private:
    QString buildHtml(double fromLng, double fromLat, double toLng, double toLat) const;

    QWebEngineView *m_view;
    QLabel          *m_statusLabel;
};

#endif // MAPVIEW_H