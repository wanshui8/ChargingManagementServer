#include "salespage.h"
#include "serverservice.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QPainter>
#include <QDate>

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>

SalesPage::SalesPage(ServerService *service, QWidget *parent)
    : QWidget(parent)
    , m_service(service)
{
    // ---- 数字卡 ----
    QLabel *todayTitle = new QLabel("今日营收 (元)");
    QLabel *monthTitle = new QLabel("本月营收 (元)");
    QLabel *totalTitle = new QLabel("总营收 (元)");

    m_todayLabel = new QLabel("0.00");
    m_monthLabel = new QLabel("0.00");
    m_totalLabel = new QLabel("0.00");

    QList<QLabel *> valueLabels = { m_todayLabel, m_monthLabel, m_totalLabel };
    for (QLabel *lbl : valueLabels) {
        QFont f = lbl->font();
        f.setPointSize(20);
        f.setBold(true);
        lbl->setFont(f);
        lbl->setStyleSheet("color: #2f7eea;");
    }

    QWidget *cardToday = new QWidget;
    QVBoxLayout *ct = new QVBoxLayout(cardToday);
    ct->addWidget(todayTitle);
    ct->addWidget(m_todayLabel);
    cardToday->setStyleSheet("background:#f5f7fa; border-radius:8px;");

    QWidget *cardMonth = new QWidget;
    QVBoxLayout *cm = new QVBoxLayout(cardMonth);
    cm->addWidget(monthTitle);
    cm->addWidget(m_monthLabel);
    cardMonth->setStyleSheet("background:#f5f7fa; border-radius:8px;");

    QWidget *cardTotal = new QWidget;
    QVBoxLayout *ct2 = new QVBoxLayout(cardTotal);
    ct2->addWidget(totalTitle);
    ct2->addWidget(m_totalLabel);
    cardTotal->setStyleSheet("background:#f5f7fa; border-radius:8px;");

    QHBoxLayout *cardLayout = new QHBoxLayout;
    cardLayout->addWidget(cardToday);
    cardLayout->addWidget(cardMonth);
    cardLayout->addWidget(cardTotal);

    // ---- 时间维度切换 + 刷新 ----
    m_btn7 = new QPushButton("近7日");
    m_btn30 = new QPushButton("近30日");
    QPushButton *refreshBtn = new QPushButton("刷新");

    QHBoxLayout *toolLayout = new QHBoxLayout;
    toolLayout->addWidget(m_btn7);
    toolLayout->addWidget(m_btn30);
    toolLayout->addStretch();
    toolLayout->addWidget(refreshBtn);

    // ---- 图表 ----
    m_chartView = new QChartView(this);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(cardLayout);
    mainLayout->addLayout(toolLayout);
    mainLayout->addWidget(m_chartView, 1);

    connect(m_service, &ServerService::revenueReady,
            this, &SalesPage::onRevenueReady);
    connect(refreshBtn, &QPushButton::clicked, this, &SalesPage::onRefresh);
    connect(m_btn7, &QPushButton::clicked, this, &SalesPage::show7Days);
    connect(m_btn30, &QPushButton::clicked, this, &SalesPage::show30Days);

    onRefresh();
}

void SalesPage::onRefresh()
{
    m_service->asyncLoadRevenue();
}

void SalesPage::show7Days()
{
    m_days = 7;
    updateChart();
}

void SalesPage::show30Days()
{
    m_days = 30;
    updateChart();
}

void SalesPage::onRevenueReady(const QList<QPair<QString, double>> &trend,
                               double today, double month, double total)
{
    m_todayLabel->setText(QString::number(today, 'f', 2));
    m_monthLabel->setText(QString::number(month, 'f', 2));
    m_totalLabel->setText(QString::number(total, 'f', 2));

    m_trend30 = trend;
    updateChart();
}

void SalesPage::updateChart()
{
    QChart *chart = new QChart;
    chart->setTitle("营收趋势");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QLineSeries *series = new QLineSeries;
    series->setName("营收 (元)");

    // 取最近 m_days 个数据点
    int start = qMax(0, m_trend30.size() - m_days);
    QStringList categories;
    double maxValue = 1.0;
    for (int i = start; i < m_trend30.size(); ++i) {
        const QPair<QString, double> &p = m_trend30.at(i);
        categories << p.first;
        series->append(i - start, p.second);
        maxValue = qMax(maxValue, p.second);
    }
    if (categories.isEmpty()) {
        categories << QDate::currentDate().toString("MM-dd");
        series->append(0, 0);
    }

    chart->addSeries(series);

    QCategoryAxis *axisX = new QCategoryAxis;
    for (int i = 0; i < categories.size(); ++i) {
        axisX->append(categories.at(i), i);
    }
    axisX->setLabelsAngle(-45);
    axisX->setGridLineVisible(false);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("元");
    axisY->setMin(0);
    axisY->setMax(maxValue * 1.2);
    axisY->setLabelFormat("%.2f");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    m_chartView->setChart(chart);   // setChart 会自动接管并释放旧图表
}