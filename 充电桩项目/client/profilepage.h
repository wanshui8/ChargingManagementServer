#ifndef PROFILEPAGE_H
#define PROFILEPAGE_H

#include <QWidget>
#include <QJsonObject>

#include "datatypes.h"

class QLabel;
class QLineEdit;
class QPushButton;
class QListWidget;
class TcpClient;
class MainWindow;

// 我的页：用户信息展示 + 修改昵称/头像 + 充值 + 我的订单
class ProfilePage : public QWidget
{
    Q_OBJECT
public:
    explicit ProfilePage(TcpClient *client, MainWindow *mainWindow, QWidget *parent = nullptr);

    void refresh();

private slots:
    void onMessageReceived(quint16 msgId, const QJsonObject &payload);
    void onRechargeClicked();
    void onSaveNicknameClicked();
    void onChangeAvatarClicked();
    void goBack();

private:
    void displayUser(const UserInfo &user);
    QPixmap defaultAvatar(const QString &nickname) const;

    TcpClient *m_client;
    MainWindow *m_mainWindow;

    QLabel      *m_avatarLabel;
    QLabel      *m_nicknameLabel;
    QLabel      *m_phoneLabel;
    QLabel      *m_balanceLabel;

    QLineEdit   *m_nicknameEdit;
    QPushButton *m_saveNicknameBtn;
    QPushButton *m_changeAvatarBtn;

    QLineEdit   *m_rechargeEdit;
    QPushButton *m_rechargeBtn;

    QListWidget *m_orderList;
};

#endif // PROFILEPAGE_H