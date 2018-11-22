#ifndef DLCTIMER_H
#define DLCTIMER_H

#include <QLCDNumber>
#include <QTimer>
#include <time.h>
#include <QTextStream>

class dlctimer : public QLCDNumber
{   
    Q_OBJECT

    private:
        int seconds;
        QTimer *timer;
    public:
        dlctimer(QWidget *parent = 0);

    private slots:
        void showTime();
        void myChangeValue(int val);
        void myStopLCD();
        void myRestart();
};

#endif // DLCTIMER_H
