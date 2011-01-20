#include <QDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    port = NULL;

    // ***************************

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();

    foreach (QextPortInfo port, ports)
    {
        if (port.friendName == deviceName)
        {
        }
    }

    // ***************************

    port = new QextSerialPort(QextSerialPort::EventDriven);
    if (port)
    {
        port->setPortName("COM1");
        port->setBaudRate(BAUD57600);
        port->setFlowControl(FLOW_OFF);
        port->setParity(PAR_NONE);
        port->setDataBits(DATA_8);
        port->setStopBits(STOP_1);

        connect(port, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));

        port->open(0);

        qDebug("isOpen : %d", port->isOpen());
    }

    // ***************************
}

MainWindow::~MainWindow()
{
    if (port)
    {
        if (port->isOpen())
            port->close();

        delete port;
        port = NULL;
    }

    delete ui;
}

void MainWindow::onDataAvailable()
{
    if (!port)
        return;

    int avail = port->bytesAvailable();
    if (avail <= 0)
        return;

    QByteArray data;
    data.resize(avail);

    int read = port->read(data.data(), data.size());
    if (read <= 0)
        return;






    qDebug("bytes available: %d", avail);
    qDebug("received: %d", read);
}

void MainWindow::closePort()
{
    if (!port)
        return;

    port->close();
    qDebug("is open: %d", port->isOpen());
}

void MainWindow::openPort()
{
    if (!port)
        return;

    port->open(QIODevice::ReadWrite);
    qDebug("is open: %d", port->isOpen());
}
