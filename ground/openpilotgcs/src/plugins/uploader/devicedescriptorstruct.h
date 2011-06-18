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
        deviceDescriptorStruct();
};

#endif // DEVICEDESCRIPTORSTRUCT_H
