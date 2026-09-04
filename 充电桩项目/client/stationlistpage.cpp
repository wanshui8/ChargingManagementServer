#include "stationlistpage.h"
#include "tcpclient.h"
#include "mainwindow.h"
#include "piledetailpage.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace {
const char *GEOCODE_API = "https://apis.map.qq.com/ws/geocoder/v1/";
const char *API_KEY = "PEPBZ-JXBL2-XKIU7-CUDDP-GT7ZE-TEF7R";
}

StationListPage::StationListPage(TcpClient *client, MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent)
    , m_client(client)
    , m_mainWindow(mainWindow)
{
    // ---- 顶部栏 ----
    QLabel *titleLabel = new QLabel("附近充电站");
    QFont f = titleLabel->font();
    f.setPointSize(16);
    f.setBold(true);
    titleLabel->setFont(f);

    QPushButton *profileBtn = new QPushButton("我的");
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(profileBtn);

    // ---- 定位区 ----
    m_areaCombo = new QComboBox;
    m_areaCombo->addItem("滨江区政府", "120.21,30.21");
    m_areaCombo->addItem("市民中心", "120.20,30.20");
    m_areaCombo->addItem("高铁东站", "120.23,30.19");
    m_areaCombo->addItem("万达广场", "120.19,30.22");
    m_areaCombo->addItem("生态公园", "120.18,30.18");
    m_areaCombo->addItem("大学城", "120.22,30.23");

    m_addressEdit = new QLineEdit;
    m_addressEdit->setPlaceholderText("或手动输入地址");

    m_locateButton = new QPushButton("定位");

    m_locationLabel = new QLabel("当前位置：滨江区政府");

    QHBoxLayout *locateLayout = new QHBoxLayout;
    locateLayout->addWidget(m_areaCombo);
    locateLayout->addWidget(m_locateButton);

    // ---- 充电站列表 ----
    m_stationList = new QListWidget;
    m_stationList->setSpacing(6);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->addLayout(topLayout);
    layout->addWidget(m_areaCombo);
    layout->addWidget(m_addressEdit);
    layout->addLayout(locateLayout);
    layout->addWidget(m_locationLabel);
    layout->addWidget(m_stationList, 1);

    m_network = new QNetworkAccessManager(this);

    connect(m_locateButton, &QPushButton::clicked, this, &StationListPage::onLocate);
    connect(profileBtn, &QPushButton::clicked, this, &StationListPage::goToProfile);
    connect(m_client, &TcpClient::messageReceived, this, &StationListPage::onMessageReceived);
    connect(m_network, &QNetworkAccessManager::finished,
            this, &StationListPage::onGeocodeFinished);
}

void StationListPage::onLocate()
{
    // 优先使用手动地址做地理编码；地址为空则用下拉区域的预设经纬度（模拟 GPS）
    QString address = m_addressEdit->text().trimmed();
    if (!address.isEmpty()) {
        QUrl url(QString(GEOCODE_API) + "?address=" + QUrl::toPercentEncoding(address) +
                 "&key=" + API_KEY);
        m_network->get(QNetworkRequest(url));
        m_locationLabel->setText("正在解析地址...");
        return;
    }

    QString coord = m_areaCombo->currentData().toString();
    QStringList parts = coord.split(',');
    m_userLng = parts.value(0).toDouble();
    m_userLat = parts.value(1).toDouble();
    m_locationLabel->setText("当前位置：" + m_areaCombo->currentText());
    requestStationList();
}

void StationListPage::onGeocodeFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }
    reply->deleteLater();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &err);
    if (err.error != QJsonParseError::NoError || doc.object().value("status").toInt() != 0) {
        m_locationLabel->setText("地址解析失败，使用默认位置");
        requestStationList();
        return;
    }

    QJsonObject location = doc.object().value("result").toObject().value("location").toObject();
    m_userLng = location.value("lng").toDouble();
    m_userLat = location.value("lat").toDouble();
    m_locationLabel->setText(QString("当前位置：(经度 %1, 纬度 %2)").arg(m_userLng).arg(m_userLat));
    requestStationList();
}

void StationListPage::requestStationList()
{
    QJsonObject payload;
    payload["longitude"] = m_userLng;
    payload["latitude"] = m_userLat;
    m_client->send(Protocol::MsgId::REQ_STATION_LIST, payload);
}

void StationListPage::onMessageReceived(quint16 msgId, const QJsonObject &payload)
{
    if (msgId != Protocol::MsgId::RES_STATION_LIST) {
        return;
    }

    QJsonArray arr = payload.value("stations").toArray();
    m_stations.clear();
    for (const QJsonValue &v : arr) {
        m_stations.append(v.toObject());
    }
    rebuildList();
}

void StationListPage::rebuildList()
{
    m_stationList->clear();
    for (const QJsonObject &s : m_stations) {
        StationInfo station;
        station.id = s.value("id").toInt();
        station.name = s.value("name").toString();
        station.address = s.value("address").toString();
        station.longitude = s.value("longitude").toDouble();
        station.latitude = s.value("latitude").toDouble();
        station.pricePerKwh = s.value("price_per_kwh").toDouble();
        station.totalPiles = s.value("total_piles").toInt();
        station.idlePiles = s.value("idle_piles").toInt();

        QListWidgetItem *item = new QListWidgetItem(m_stationList);

        QWidget *card = new QWidget;
        QLabel *nameLabel = new QLabel(station.name);
        QFont nf = nameLabel->font();
        nf.setPointSize(13);
        nf.setBold(true);
        nameLabel->setFont(nf);

        QLabel *addrLabel = new QLabel(station.address);
        addrLabel->setStyleSheet("color: gray;");

        QLabel *infoLabel = new QLabel(QString("价格 %1 元/度    空闲 %2/%3")
                                           .arg(station.pricePerKwh)
                                           .arg(station.idlePiles)
                                           .arg(station.totalPiles));

        QLabel *distLabel = new QLabel(QString::number(s.value("distance").toDouble(), 'f', 2) + " km");
        distLabel->setStyleSheet("color: #2f7eea; font-weight: bold;");

        QPushButton *detailBtn = new QPushButton("详情");
        QPushButton *navBtn = new QPushButton("导航");
        detailBtn->setFixedWidth(54);
        navBtn->setFixedWidth(54);

        // 进入电桩详情
        connect(detailBtn, &QPushButton::clicked, this, [this, station]() {
            m_mainWindow->showStationDetail(station);
        });
        // 一键导航（起点：当前模拟 GPS，终点：目标电站）
        connect(navBtn, &QPushButton::clicked, this, [this, station]() {
            m_mainWindow->showMap(m_userLng, m_userLat, station.longitude, station.latitude);
        });

        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        QHBoxLayout *row1 = new QHBoxLayout;
        row1->addWidget(nameLabel, 1);
        row1->addWidget(detailBtn);
        row1->addWidget(navBtn);
        QHBoxLayout *row2 = new QHBoxLayout;
        row2->addWidget(infoLabel);
        row2->addStretch();
        row2->addWidget(distLabel);
        cardLayout->addLayout(row1);
        cardLayout->addWidget(addrLabel);
        cardLayout->addLayout(row2);
        card->setStyleSheet("background: #f7f9fc; border-radius: 8px; padding: 6px;");

        item->setSizeHint(card->sizeHint());
        m_stationList->setItemWidget(item, card);
    }
}

void StationListPage::goToProfile()
{
    m_mainWindow->switchTo(MainWindow::PageProfile);
}