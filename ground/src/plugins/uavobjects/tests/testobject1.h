#ifndef TESTOBJECT1_H
#define TESTOBJECT1_H

#include "..\uavdataobject.h"

class TestObject1: public UAVDataObject
{
public:
    typedef struct {
        qint16 field1;
        float field2[3];
        qint8 field3;
        qint32 field4;
    } __attribute__((packed)) DataFields;

    static const quint32 OBJID = 0x0005;
    static const QString NAME;
    static const bool SINGLEINST = false;
    static const quint32 NUMBYTES = sizeof(DataFields);

    TestObject1();

    DataFields getData();
    void setData(DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

private:
    DataFields data;

};

#endif // TESTOBJECT1_H
