#include "profilepage.h"
#include "tcpclient.h"
#include "mainwindow.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QPixmap>
#include <QPainter>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QByteArray>

ProfilePage::ProfilePage(TcpClient *client, MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent)
    , m_client(client)
    , m_mainWindow(mainWindow)
{
    // ---- 顶部栏 ----
    QPushButton *backBtn = new QPushButton("返回");
    QLabel *titleLabel = new QLabel("我的");
    QFont tf = titleLabel->font();
    tf.setPointSize(16);
    tf.setBold(true);
    titleLabel->setFont(tf);

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(backBtn);
    topLayout->addSpacing(8);
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();

    // ---- 用户信息区 ----
    m_avatarLabel = new QLabel;
    m_avatarLabel->setFixedSize(72, 72);

    m_nicknameLabel = new QLabel;
    m_nicknameLabel->setStyleSheet("font-size:16px; font-weight:bold;");
    m_phoneLabel = new QLabel;
    m_phoneLabel->setStyleSheet("color:gray;");
    m_balanceLabel = new QLabel;
    m_balanceLabel->setStyleSheet("color:#2f7eea; font-weight:bold;");

    QVBoxLayout *infoLayout = new QVBoxLayout;
    infoLayout->addWidget(m_nicknameLabel);
    infoLayout->addWidget(m_phoneLabel);
    infoLayout->addWidget(m_balanceLabel);

    QHBoxLayout *userLayout = new QHBoxLayout;
    userLayout->addWidget(m_avatarLabel);
    userLayout->addSpacing(12);
    userLayout->addLayout(infoLayout);
    userLayout->addStretch();

    // ---- 修改昵称 / 头像 ----
    m_nicknameEdit = new QLineEdit;
    m_nicknameEdit->setPlaceholderText("输入新昵称");
    m_saveNicknameBtn = new QPushButton("保存昵称");
    m_changeAvatarBtn = new QPushButton("更换头像");

    QHBoxLayout *nickLayout = new QHBoxLayout;
    nickLayout->addWidget(m_nicknameEdit, 1);
    nickLayout->addWidget(m_saveNicknameBtn);
    nickLayout->addWidget(m_changeAvatarBtn);

    // ---- 充值 ----
    m_rechargeEdit = new QLineEdit;
    m_rechargeEdit->setPlaceholderText("充值金额(元)");
    m_rechargeBtn = new QPushButton("充值");

    QHBoxLayout *rechargeLayout = new QHBoxLayout;
    rechargeLayout->addWidget(m_rechargeEdit, 1);
    rechargeLayout->addWidget(m_rechargeBtn);

    // ---- 我的订单 ----
    QLabel *orderTitle = new QLabel("我的订单");
    orderTitle->setStyleSheet("font-weight:bold;");
    m_orderList = new QListWidget;

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->addLayout(topLayout);
    layout->addSpacing(8);
    layout->addLayout(userLayout);
    layout->addSpacing(12);
    layout->addLayout(nickLayout);
    layout->addSpacing(8);
    layout->addLayout(rechargeLayout);
    layout->addSpacing(12);
    layout->addWidget(orderTitle);
    layout->addWidget(m_orderList, 1);

    connect(backBtn, &QPushButton::clicked, this, &ProfilePage::goBack);
    connect(m_saveNicknameBtn, &QPushButton::clicked, this, &ProfilePage::onSaveNicknameClicked);
    connect(m_changeAvatarBtn, &QPushButton::clicked, this, &ProfilePage::onChangeAvatarClicked);
    connect(m_rechargeBtn, &QPushButton::clicked, this, &ProfilePage::onRechargeClicked);
    connect(m_client, &TcpClient::messageReceived, this, &ProfilePage::onMessageReceived);
}

void ProfilePage::refresh()
{
    displayUser(m_client->currentUser());

    QJsonObject payload;
    payload["user_id"] = m_client->currentUser().id;
    m_client->send(Protocol::MsgId::REQ_MY_ORDERS, payload);
}

void ProfilePage::displayUser(const UserInfo &user)
{
    m_nicknameLabel->setText(user.nickname);
    m_phoneLabel->setText(user.phone);
    m_balanceLabel->setText(QString("余额：%1 元").arg(user.balance, 0, 'f', 2));
    m_nicknameEdit->setText(user.nickname);

    if (!user.avatar.isEmpty()) {
        QByteArray bytes = QByteArray::fromBase64(user.avatar.toLatin1());
        QPixmap pix;
        pix.loadFromData(bytes);
        m_avatarLabel->setPixmap(pix.scaled(72, 72, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_avatarLabel->setPixmap(defaultAvatar(user.nickname));
    }
}

QPixmap ProfilePage::defaultAvatar(const QString &nickname) const
{
    QPixmap pix(72, 72);
    pix.fill(QColor(220, 220, 220));
    QPainter painter(&pix);
    painter.setPen(QColor(120, 120, 120));
    QFont f = painter.font();
    f.setPointSize(28);
    f.setBold(true);
    painter.setFont(f);
    painter.drawText(pix.rect(), Qt::AlignCenter, nickname.left(1));
    return pix;
}

void ProfilePage::onMessageReceived(quint16 msgId, const QJsonObject &payload)
{
    switch (msgId) {
    case Protocol::MsgId::RES_RECHARGE: {
        if (payload.value("code").toInt() == Protocol::ErrorCode::OK) {
            double balance = payload.value("balance").toDouble();
            m_balanceLabel->setText(QString("余额：%1 元").arg(balance, 0, 'f', 2));
            UserInfo u = m_client->currentUser();
            u.balance = balance;
            m_client->setCurrentUser(u);
            QMessageBox::information(this, "充值成功", "充值成功");
        }
        break;
    }
    case Protocol::MsgId::RES_UPDATE_PROFILE: {
        if (payload.value("code").toInt() == Protocol::ErrorCode::OK) {
            QJsonObject userObj = payload.value("user").toObject();
            UserInfo u;
            u.id = userObj.value("id").toInt();
            u.phone = userObj.value("phone").toString();
            u.nickname = userObj.value("nickname").toString();
            u.avatar = userObj.value("avatar").toString();
            u.balance = userObj.value("balance").toDouble();
            u.status = userObj.value("status").toString();
            m_client->setCurrentUser(u);
            displayUser(u);
        }
        break;
    }
    case Protocol::MsgId::RES_MY_ORDERS: {
        QJsonArray arr = payload.value("orders").toArray();
        m_orderList->clear();
        for (const QJsonValue &v : arr) {
            QJsonObject o = v.toObject();
            QString statusCn = (o.value("status").toString() == "charging") ? "充电中" : "已结算";
            QString text = QString("订单 %1 | 电桩 %2 | %3 | %4 kWh | %5 元 | %6")
                               .arg(o.value("id").toInt())
                               .arg(o.value("pile_id").toInt())
                               .arg(statusCn)
                               .arg(o.value("kwh").toDouble(), 0, 'f', 2)
                               .arg(o.value("fee").toDouble(), 0, 'f', 2)
                               .arg(o.value("start_time").toString());
            m_orderList->addItem(text);
        }
        break;
    }
    default:
        break;
    }
}

void ProfilePage::onRechargeClicked()
{
    bool ok = false;
    double amount = m_rechargeEdit->text().toDouble(&ok);
    if (!ok || amount <= 0) {
        QMessageBox::warning(this, "提示", "请输入有效的充值金额");
        return;
    }
    QJsonObject payload;
    payload["user_id"] = m_client->currentUser().id;
    payload["amount"] = amount;
    m_client->send(Protocol::MsgId::REQ_RECHARGE, payload);
    m_rechargeEdit->clear();
}

void ProfilePage::onSaveNicknameClicked()
{
    QString nickname = m_nicknameEdit->text().trimmed();
    if (nickname.isEmpty()) {
        QMessageBox::warning(this, "提示", "昵称不能为空");
        return;
    }
    QJsonObject payload;
    payload["user_id"] = m_client->currentUser().id;
    payload["nickname"] = nickname;
    payload["avatar"] = m_client->currentUser().avatar;
    m_client->send(Protocol::MsgId::REQ_UPDATE_PROFILE, payload);
}

void ProfilePage::onChangeAvatarClicked()
{
    QString path = QFileDialog::getOpenFileName(this, "选择头像", QString(),
                                                "图片文件 (*.png *.jpg *.jpeg *.bmp)");
    if (path.isEmpty()) {
        return;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "提示", "无法读取图片");
        return;
    }
    QByteArray bytes = file.readAll();
    file.close();

    // 限制头像大小，避免单条消息过大
    if (bytes.size() > 200 * 1024) {
        QMessageBox::warning(this, "提示", "图片过大，请选择 200KB 以内的图片");
        return;
    }

    QString base64 = QString::fromLatin1(bytes.toBase64());
    QJsonObject payload;
    payload["user_id"] = m_client->currentUser().id;
    payload["nickname"] = m_client->currentUser().nickname;
    payload["avatar"] = base64;
    m_client->send(Protocol::MsgId::REQ_UPDATE_PROFILE, payload);
}

void ProfilePage::goBack()
{
    m_mainWindow->switchTo(MainWindow::PageStationList);
}