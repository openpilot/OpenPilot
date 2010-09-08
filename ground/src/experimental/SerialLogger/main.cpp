#include <QtCore/QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <qextserialport.h>

#include <iostream>

class pollSerialPort : public QThread
{
public:
    pollSerialPort(QString dn, QString fn) : device(dn), outFile(fn)
    {
    };

    void run()
    {
        QByteArray dat;
        //const char framingRaw[16] = {7,9,3,15,193,130,150,10,7,9,3,15,193,130,150,10};
        const char framingRaw[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
        QByteArray framing(framingRaw,16);

        PortSettings Settings;
        Settings.BaudRate=BAUD115200;
        Settings.DataBits=DATA_8;
        Settings.Parity=PAR_NONE;
        Settings.StopBits=STOP_1;
        Settings.FlowControl=FLOW_OFF;
        Settings.Timeout_Millisec=500;

        QextSerialPort serialPort(device, Settings);
        serialPort.open(QIODevice::ReadOnly);

        QFile file(outFile);
        if( !file.open( QIODevice::WriteOnly ) )
        {
            qDebug() << "Failed to open file: " << outFile;
          return;
        }

        QTextStream ts( &file );

        while(1)
        {
            dat = serialPort.read(500);
            if(dat.contains(framing))
            {
                int start = dat.indexOf(framing);
                int count = *((int *) (dat.data() + start+16));
                qDebug() << "Found frame start at " << start << " count " << count;
            }
            else if (dat.size() == 0)
                qDebug() << "No data";
            else
                qDebug() << "No frame start";
            ts << dat;
            usleep(100000);
        }
    };

protected:
    QString device;
    QString outFile;
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString device;
    QString log;

    if(argc < 2)
        device = "/dev/tty.usbserial-000014FAB";
    else
        device = QString(argv[1]);

    if(argc < 3)
        log = "log.dat";
    else
        log = QString(argv[2]);

    pollSerialPort thread(device, log);
    thread.start();
    return a.exec();
}
