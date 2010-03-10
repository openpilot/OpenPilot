#ifndef UAVMETAOBJECT_H
#define UAVMETAOBJECT_H

#include "uavobject.h"

class UAVMetaObject: public UAVObject
{
    Q_OBJECT

public:
    UAVMetaObject(quint32 objID, QString& name, Metadata& mdata, UAVObject* parent);
    UAVObject* getParentObject();

    qint32 pack(quint8* dataOut);
    qint32 unpack(const quint8* dataIn);
    void setMetadata(const Metadata& mdata);
    Metadata getMetadata();
    void setData(const Metadata& mdata);
    Metadata getData();

private:
    UAVObject* parent;
    Metadata ownMetadata;
    Metadata parentMetadata;

};

#endif // UAVMETAOBJECT_H
