#ifndef USBSIGNALFILTER_H
#define USBSIGNALFILTER_H
#include <QObject>
#include "usbmonitor.h"

class RAWHID_EXPORT USBSignalFilter : public QObject
{
    Q_OBJECT
private:
    int m_vid;
    int m_pid;
    int m_boardModel;
    int m_runState;
signals:
    void deviceDiscovered();
private slots:
    void m_deviceDiscovered(USBPortInfo port);
public:
    USBSignalFilter(int vid, int pid, int boardModel, int runState);
};
#endif // USBSIGNALFILTER_H
