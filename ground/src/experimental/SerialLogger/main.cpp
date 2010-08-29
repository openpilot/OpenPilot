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
        PortSettings Settings;
        Settings.BaudRate=BAUD57600;
        Settings.DataBits=DATA_8;
        Settings.Parity=PAR_NONE;
        Settings.StopBits=STOP_1;
        Settings.FlowControl=FLOW_HARDWARE;
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
            dat = serialPort.read(100);
            qDebug() << dat;
            ts << dat;
        }
    };

protected:
    QString device;
    QString outFile;
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    pollSerialPort thread("/dev/tty.usbserial-000014FAB","log.dat"); //argv[0]);
    thread.start();
    return a.exec();
}
