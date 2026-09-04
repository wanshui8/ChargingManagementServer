#ifndef USERMANAGEPAGE_H
#define USERMANAGEPAGE_H

#include <QWidget>
#include <QList>

#include "database.h"

class ServerService;
class QTableWidget;
class QLineEdit;
class QPushButton;

// 用户管理页：搜索 + 冻结/解冻
class UserManagePage : public QWidget
{
    Q_OBJECT
public:
    explicit UserManagePage(ServerService *service, QWidget *parent = nullptr);

private slots:
    void onUsersReady(const QList<UserInfo> &users);
    void onRefresh();
    void onToggleStatus();
    void onSearchTextChanged(const QString &text);

private:
    void rebuildTable();

    ServerService *m_service;
    QTableWidget   *m_table;
    QLineEdit      *m_searchEdit;
    QPushButton    *m_toggleButton;

    QList<UserInfo> m_users;
    QString m_filter;
};

#endif // USERMANAGEPAGE_H