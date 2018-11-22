#include "chartview.h"
#include <QtCore/QtMath>
#include <QtCore/QDebug>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtCore/QString>
#include <QTimer>
#include <time.h>
#include <sstream>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <vector>
#include <QTextStream>
#include <QLabel>
#include <QSize>
#include <QPushButton>
#include <QButtonGroup>

QT_CHARTS_USE_NAMESPACE

chartview::chartview(QWidget *parent) :
    QChartView(parent)
{
    chartview::interval = time (NULL);
    slider=false;
    beginnig = time(NULL);
    QTextStream out(stdout);
    float maxX=0;
    float maxY=0;
    device dev;
    setRenderHint(QPainter::Antialiasing);

    chart()->setTitle("Click to interact with scatter points");


    timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(updateChart()));
        timer->start(1000);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        //db.setHostName("http://localhost/phpmyadmin");
        db.setDatabaseName("C:/Users/TheCreator/Documents/UIPositions/timestamp_positions.db");
        //db.setUserName("");
        //db.setPassword("");
        bool ok = db.open();
        if(!ok){
             out << "not connected" << endl;
        }
    QSqlQuery qry;

    if (qry.exec("SELECT * FROM devices"))
    {
       while(qry.next())
       {
           dev.setX(qry.value(1).toFloat());
           dev.setId(qry.value(0).toInt());
           dev.setY(qry.value(2).toFloat());
           devices.push_back(dev);
       }
    }
    else
    {
        qDebug() << qry.lastError();
    }
    for(device d : devices){
        if(d.getX()>maxX){
            maxX=d.getX();
        }
        if(d.getY()>maxY){
            maxY=d.getX();
        }
    }

    m_scatter = new QScatterSeries();
    m_scatter->setName("scatter1");
    chart()->addSeries(m_scatter);
    chart()->createDefaultAxes();
    chart()->axisX()->setRange(0, maxX+5); //todo inserire costante giusta
    chart()->axisY()->setRange(0, maxY+5);
    connect(m_scatter, &QScatterSeries::clicked, this, &chartview::handleClickedPoint);
}

void chartview::handleClickedPoint(const QPointF &point)
{
    QPointF clickedPoint = point;
    QSqlQuery qry;
    // Find the closest point from series 1
    QPointF closest(INT_MAX, INT_MAX);
    qreal distance(INT_MAX);
    const auto points = m_scatter->points();
    for (const QPointF &currentPoint : points) {
        qreal currentDistance = qSqrt((currentPoint.x() - clickedPoint.x())
                                      * (currentPoint.x() - clickedPoint.x())
                                      + (currentPoint.y() - clickedPoint.y())
                                      * (currentPoint.y() - clickedPoint.y()));
        if (currentDistance < distance) {
            distance = currentDistance;
            closest = currentPoint;
        }
    }
    QString query= "SELECT * FROM positions WHERE x==" +
                    QString::number(closest.x()) + " AND y==" +
                    QString::number(closest.y());
    QTextStream out(stdout);
    out << query << endl;
    QWidget *wdg = new QWidget;
    wdg->resize(400,300);
    QLabel *label= new QLabel(wdg);
    if (qry.exec(query))
    {
         qry.first();
         label->setText("MAC: "+ qry.value(0).toString() +".\n"
                        "X: "+ qry.value(1).toString() +".\n"
                        "Y: "+ qry.value(2).toString() +".\n"
                        "RSSI: "+ qry.value(3).toString() +".\n"
                        "SSID: "+ qry.value(4).toString() +".\n"
                        "MAC: "+ qry.value(5).toString() +".\n"
                        "TIMESTAMP: "+ qry.value(6).toString() +".\n"
                        "SEQUENCE NUMBER: "+ qry.value(7).toString() +".\n");
    }
    else
    {
        qDebug() << qry.lastError();
    }

    wdg->show();
}

void chartview::updateChart(){
    m_scatter->clear();
    QSqlQuery qry;
    position p;
    QTextStream out(stdout);
    QString query;
    if(!slider){
        time_t now = time(NULL);
        query = "SELECT * FROM positions";
                        //WHERE timestamp<" +
                        //QString::number(now) + " AND timestamp>" +
                        //QString::number(ChartView::beginning);
        chartview::interval= now;
    }
    else{
        query= "SELECT * FROM positions";
                        //WHERE timestamp<" +
                        //QString::number(interval) + " AND timestamp>" +
                        //QString::number(ChartView::beginning);
    }
    if (qry.exec(query))
    {
       while(qry.next())
       {
            p.setHash(qry.value(0).toString());
            p.setX(qry.value(1).toFloat());
            p.setY(qry.value(2).toFloat());
            p.setRssi(qry.value(3).toInt());
            p.setSsid(qry.value(4).toString());
            p.setMac(qry.value(5).toString());
            //ChartView::positions[i].setChannel(qry.value(6).toInt());
            p.setTimestamp(qry.value(7).toInt());
            p.setSequence_number(qry.value(8).toInt());
            positions.push_back(p);
       }
    }
    else
    {
        qDebug() << qry.lastError();
    }
    for(position p : positions){
        *m_scatter << QPointF( p.position::getX(), p.position::getY());

    }
    positions.clear();
}

void chartview::myRestart()
{
    updateChart();
    QTextStream out(stdout);
    out << "timer started." << endl;
    timer->start();
}

void chartview::myStop()
{
    timer->stop();
    QTextStream out(stdout);
    out << "timer stopped." << endl;
}

void chartview::myChangeValue(int val)
{
    timer->stop();
    interval-= val;
    slider=true;
    updateChart();
}

chartview::~chartview()
{

}

