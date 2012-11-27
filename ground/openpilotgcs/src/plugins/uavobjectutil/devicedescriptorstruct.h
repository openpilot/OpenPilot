#ifndef DEVICEDESCRIPTORSTRUCT_H
#define DEVICEDESCRIPTORSTRUCT_H

#include <QString>
struct deviceDescriptorStruct
{
public:
       QString gitHash;
       QString gitDate;
       QString gitTag;
       QByteArray fwHash;
       QByteArray uavoHash;
       int boardType;
       int boardRevision;
       static QString idToBoardName(int id)
       {
           switch (id) {
           case 0x0101://MB
               return QString("OpenPilot MainBoard");
               break;
           case 0x0201://INS
               return QString("OpenPilot INS");
               break;
           case 0x0301://PipX
               return QString("OPLink");
               break;
           case 0x0401://Coptercontrol
               return QString("CopterControl");
               break;
           case 0x0402://Coptercontrol
               // It would be nice to say CC3D here but since currently we use string comparisons
               // for firmware compatibility and the filename path that would break
               return QString("CopterControl");
               break;
           default:
               return QString("");
               break;
           }
       }
        deviceDescriptorStruct(){}
};

#endif // DEVICEDESCRIPTORSTRUCT_H
