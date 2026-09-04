#include "usermanagepage.h"
#include "serverservice.h"

#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

UserManagePage::UserManagePage(ServerService *service, QWidget *parent)
    : QWidget(parent)
    , m_service(service)
{
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("按手机号搜索...");

    QPushButton *refreshBtn = new QPushButton("刷新");
    m_toggleButton = new QPushButton("冻结/解冻");
    m_toggleButton->setEnabled(false);

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(m_searchEdit);
    topLayout->addWidget(refreshBtn);
    topLayout->addWidget(m_toggleButton);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({ "用户ID", "手机号", "昵称", "余额(元)", "状态", "注册时间" });
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_table);

    connect(m_service, &ServerService::usersReady,
            this, &UserManagePage::onUsersReady);
    connect(refreshBtn, &QPushButton::clicked, this, &UserManagePage::onRefresh);
    connect(m_toggleButton, &QPushButton::clicked, this, &UserManagePage::onToggleStatus);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &UserManagePage::onSearchTextChanged);
    connect(m_table, &QTableWidget::itemSelectionChanged,
            this, [this]() { m_toggleButton->setEnabled(m_table->currentRow() >= 0); });

    onRefresh();
}

void UserManagePage::onRefresh()
{
    m_service->asyncLoadUsers();
}

void UserManagePage::onUsersReady(const QList<UserInfo> &users)
{
    m_users = users;
    rebuildTable();
}

void UserManagePage::onSearchTextChanged(const QString &text)
{
    m_filter = text.trimmed();
    rebuildTable();
}

void UserManagePage::rebuildTable()
{
    QList<UserInfo> filtered;
    for (const UserInfo &u : m_users) {
        if (m_filter.isEmpty() || u.phone.contains(m_filter) || u.nickname.contains(m_filter)) {
            filtered.append(u);
        }
    }

    m_table->setRowCount(filtered.size());
    for (int i = 0; i < filtered.size(); ++i) {
        const UserInfo &u = filtered.at(i);
        m_table->setItem(i, 0, new QTableWidgetItem(QString::number(u.id)));
        m_table->setItem(i, 1, new QTableWidgetItem(u.phone));
        m_table->setItem(i, 2, new QTableWidgetItem(u.nickname));
        m_table->setItem(i, 3, new QTableWidgetItem(QString::number(u.balance, 'f', 2)));
        m_table->setItem(i, 4, new QTableWidgetItem(u.status == "normal" ? "正常" : "冻结"));
        m_table->setItem(i, 5, new QTableWidgetItem(u.createTime));

        m_table->item(i, 0)->setData(Qt::UserRole, u.id);
        m_table->item(i, 1)->setData(Qt::UserRole + 1, u.status);
    }
}

void UserManagePage::onToggleStatus()
{
    int row = m_table->currentRow();
    if (row < 0) {
        return;
    }
    int userId = m_table->item(row, 0)->data(Qt::UserRole).toInt();
    QString currentStatus = m_table->item(row, 1)->data(Qt::UserRole + 1).toString();

    QString newStatus = (currentStatus == "normal") ? "frozen" : "normal";
    m_service->asyncSetUserStatus(userId, newStatus);
}