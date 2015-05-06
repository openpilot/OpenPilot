#ifndef DEVICEDESCRIPTORSTRUCT_H
#define DEVICEDESCRIPTORSTRUCT_H

#include <QString>
struct deviceDescriptorStruct {
public:
    QString        gitHash;
    QString        gitDate;
    QString        gitTag;
    QByteArray     fwHash;
    QByteArray     uavoHash;
    int boardType;
    int boardRevision;
    static QString idToBoardName(int id)
    {
        switch (id) {
         case 0x0301:
            // OPLink Mini
            return QString("OPLink");
        case 0x0901:
            // Revolution
            return QString("Revolution");
        case 0x0903:
            // Revo Mini
            return QString("Revolution");
        case 0x0904:
            return QString("DiscoveryF4");
        default:
            return QString("");
        }
    }
    deviceDescriptorStruct() {}
};

#endif // DEVICEDESCRIPTORSTRUCT_H
