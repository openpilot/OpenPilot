#ifndef UAVDATAOBJECT_H
#define UAVDATAOBJECT_H

#include "uavobject.h"
#include "uavobjectfield.h"
#include "uavmetaobject.h"
#include <QList>

class UAVDataObject: public UAVObject
{
    Q_OBJECT

public:
    UAVDataObject(quint32 objID, quint32 instID, bool isSingleInst, QString& name, quint32 numBytes);
    void initialize(QList<UAVObjectField*> fields, Metadata& mdata);
    void initialize(QList<UAVObjectField*> fields, UAVMetaObject* mobj);

    qint32 getNumFields();
    QList<UAVObjectField*> getFields();
    UAVObjectField* getField(QString& name);
    qint32 pack(quint8* dataOut);
    qint32 unpack(const quint8* dataIn);
    void setMetadata(const Metadata& mdata);
    Metadata getMetadata();
    UAVMetaObject* getMetaObject();

private slots:
    void fieldUpdated(UAVObjectField* field);

private:
    quint8* data;
    QList<UAVObjectField*> fields;
    UAVMetaObject* mobj;

    void initialize(QList<UAVObjectField*> fields);


};

#endif // UAVDATAOBJECT_H
