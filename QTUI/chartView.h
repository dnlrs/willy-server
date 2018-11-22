#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H
#include <QTCharts/QChartView>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QDateTime>
#include "device.h"
#include "position.h"
#include <vector>

QT_CHARTS_USE_NAMESPACE

class chartview : public QChartView
{
    Q_OBJECT

public:
    chartview(QWidget *parent = 0);
    ~chartview();

public slots:
    void myStop();
    void myChangeValue(int);
    void myRestart();
    void myStats();
    void myStatsStart(QDateTime);
    void myStatsStop(QDateTime);

private Q_SLOTS:
    void handleClickedPoint(const QPointF &point);
    void updateChart();

private:
    QScatterSeries *m_scatter;
    std::vector<device> devices;
    std::vector<position> positions;
    time_t interval;
    time_t beginning;
    QDateTime statStart;
    QDateTime statStop;
    QTimer *timer;
    bool slider;
    bool stat;
};

#endif // GRAPHICSVIEW_H
