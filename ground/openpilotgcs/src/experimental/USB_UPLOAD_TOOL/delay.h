#ifndef DELAY_H
#define DELAY_H
#include <QThread>

class delay : public QThread
{
public:
    static void msleep(unsigned long msecs)
    {
        QThread::msleep(msecs);
    }
};
#endif // DELAY_H
