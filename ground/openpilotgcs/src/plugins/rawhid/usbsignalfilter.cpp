#include "usbsignalfilter.h"
#include <QDebug>
void USBSignalFilter::m_deviceDiscovered(USBPortInfo port)
{
    if((port.vendorID==m_vid || m_vid==-1) && (port.productID==m_pid || m_pid==-1) && ((port.bcdDevice>>8)==m_boardModel || m_boardModel==-1) &&
            ( (port.bcdDevice&0x00ff) ==m_runState || m_runState==-1))
    {
        qDebug()<<"USBSignalFilter emit device discovered";
        emit deviceDiscovered();
    }
}

USBSignalFilter::USBSignalFilter(int vid, int pid, int boardModel, int runState):
    m_vid(vid),
    m_pid(pid),
    m_boardModel(boardModel),
    m_runState(runState)
{
    connect(USBMonitor::instance(),SIGNAL(deviceDiscovered(USBPortInfo)),this,SLOT(m_deviceDiscovered(USBPortInfo)));
}


