#include "mapview.h"

#include <QWebEngineView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace {
const char *API_KEY = "PEPBZ-JXBL2-XKIU7-CUDDP-GT7ZE-TEF7R";
}

MapView::MapView(QWidget *parent)
    : QWidget(parent)
{
    QPushButton *backBtn = new QPushButton("返回");
    QLabel *titleLabel = new QLabel("一键导航");
    QFont tf = titleLabel->font();
    tf.setPointSize(16);
    tf.setBold(true);
    titleLabel->setFont(tf);

    m_statusLabel = new QLabel("正在加载地图...");
    m_statusLabel->setStyleSheet("color:gray;");

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(backBtn);
    topLayout->addSpacing(8);
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_statusLabel);

    m_view = new QWebEngineView(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(topLayout);
    layout->addWidget(m_view, 1);

    connect(backBtn, &QPushButton::clicked, this, &MapView::backRequested);
}

void MapView::navigate(double fromLng, double fromLat, double toLng, double toLat)
{
    m_statusLabel->setText("正在规划路线...");
    QString html = buildHtml(fromLng, fromLat, toLng, toLat);
    // baseUrl 设为腾讯地图域名，便于加载 JS API 资源
    m_view->setHtml(html, QUrl("https://map.qq.com/"));
}

QString MapView::buildHtml(double fromLng, double fromLat, double toLng, double toLat) const
{
    QString html = QStringLiteral(
        "<!DOCTYPE html><html><head><meta charset='utf-8'>"
        "<style>html,body,#map{width:100%%;height:100%%;margin:0;padding:0;}"
        "#info{position:absolute;top:10px;left:10px;background:white;padding:8px;"
        "border-radius:4px;font-size:13px;z-index:999;box-shadow:0 1px 4px rgba(0,0,0,.2);}</style>"
        "<script charset='utf-8' src='https://map.qq.com/api/gljs?v=1.exp&key=%1'></script>"
        "</head><body>"
        "<div id='map'></div><div id='info'>正在规划路线...</div>"
        "<script>"
        "var FROM_LNG=%2,FROM_LAT=%3,TO_LNG=%4,TO_LAT=%5;"
        "var map=new TMap.Map('map',{center:new TMap.LatLng((FROM_LAT+TO_LAT)/2,(FROM_LNG+TO_LNG)/2),zoom:13});"
        "map.on('load',function(){planRoute();});"
        "function planRoute(){"
        "  var from=new TMap.LatLng(FROM_LAT,FROM_LNG);"
        "  var to=new TMap.LatLng(TO_LAT,TO_LNG);"
        "  new TMap.MultiMarker({map:map,geometries:[{id:'from',position:from},{id:'to',position:to}]});"
        "  var dir=new TMap.service.Direction({key:'%1'});"
        "  dir.routePlan({from:{lat:FROM_LAT,lng:FROM_LNG},to:{lat:TO_LAT,lng:TO_LNG},mode:'driving'})"
        "    .then(function(result){"
        "      var paths=result.routes[0].polyline;"
        "      var path=paths.map(function(p){return new TMap.LatLng(p.lat,p.lng);});"
        "      new TMap.MultiPolyline({map:map,styles:{s:{color:'#2f7eea',width:6}},"
        "        geometries:[{id:'r',styleId:'s',paths:path}]});"
        "      document.getElementById('info').innerText='路线规划完成，距离 '+(result.routes[0].distance/1000).toFixed(2)+' km';"
        "      map.fitBounds([from,to],{padding:60});"
        "    })"
        "    .catch(function(e){document.getElementById('info').innerText='路线规划失败，已显示起终点';});"
        "}"
        "</script></body></html>");

    return html.arg(QString::fromLatin1(API_KEY))
               .arg(fromLng, 0, 'f', 6)
               .arg(fromLat, 0, 'f', 6)
               .arg(toLng, 0, 'f', 6)
               .arg(toLat, 0, 'f', 6);
}