#include "dlctimer.h"

dlctimer::dlctimer(QWidget *parent) : QLCDNumber(parent)
{
    seconds=0;
    setSegmentStyle(Filled);
    //time_t seconds= time(NULL);
    timer = new QTimer(this);
         connect(timer, SIGNAL(timeout()), this, SLOT(showTime()));
        timer->start(1000);

    showTime();

    setWindowTitle(tr("Digital Clock"));
    resize(150, 60);
}

void dlctimer::showTime()
{
    seconds++;
    display(seconds);
}

void dlctimer::myChangeValue(int val)
{
    timer->stop();
    seconds-=val;
    QTextStream out(stdout);
    out << seconds << endl;
    display(seconds);
}

void dlctimer::myStopLCD()
{
    timer->stop();
}

void dlctimer::myRestart()
{
    seconds=0;
    timer->start();
}
