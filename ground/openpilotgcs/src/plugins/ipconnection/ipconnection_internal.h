#ifndef IPCONNECTION_INTERNAL_H
#define IPCONNECTION_INTERNAL_H

#include "ipconnectionplugin.h"

//Simple class for creating & destroying a socket in the real-time thread
//Needed because sockets need to be created in the same thread that they're used
class IPConnection : public QObject
{
    Q_OBJECT

public:

    IPConnection(IPconnectionConnection *connection);
    //virtual ~IPConnection();

public slots:

    void onOpenDevice(QString HostName, int Port, bool UseTCP);
    void onCloseDevice(QAbstractSocket *ipSocket);
};

#endif // IPCONNECTION_INTERNAL_H
