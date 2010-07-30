#include <QtCore/QCoreApplication>
#include <QThread>
#include <../../plugins/rawhid/pjrc_rawhid.h>

#define BUF_LEN 64

class MyThread : public QThread {

    public:

        void run()
        {
            qDebug() << "Hello";
            pjrc_rawhid hidHandle;
            int numDevices = hidHandle.open(1,0x20a0,0x4117,0,0); //0xff9c,0x0001);
            if( numDevices == 0 )
                numDevices = hidHandle.open(1,0x0483,0,0,0);

            qDebug() << numDevices << " device(s) opened";

            //hidHandle.mytest(0);

            char buf[BUF_LEN];
            buf[0] = 2;
            buf[1] = 0;
            int result = hidHandle.send(0,buf, BUF_LEN, 500);

            qDebug() << result << " bytes sent";

            int received = hidHandle.receive(0, buf, BUF_LEN, 3500);

            qDebug("%u bytes received.  First value %x second %x", received,buf[0], buf[1]);
        }

    };


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MyThread b;
    b.start();

    return a.exec();
}
