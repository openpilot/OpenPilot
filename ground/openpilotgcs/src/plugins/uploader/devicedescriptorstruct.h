#ifndef DEVICEDESCRIPTORSTRUCT_H
#define DEVICEDESCRIPTORSTRUCT_H

#include <QString>
struct deviceDescriptorStruct
{
public:
       QString gitTag;
       QString buildDate;
       QString description;
       int boardType;
       int boardRevision;
       static QString idToBoardName(int id)
       {
           switch (id | 0x0011) {
           case 0x0111://MB
               return QString("OpenPilot MainBoard");
               break;
           case 0x0311://PipX
               return QString("PipXtreame");
               break;
           case 0x0411://Coptercontrol
               return QString("CopterControl");
               break;
           case 0x0211://INS
               return QString("OpenPilot INS");
               break;
           default:
               return QString("");
               break;
           }
       }
        deviceDescriptorStruct();
};

#endif // DEVICEDESCRIPTORSTRUCT_H
