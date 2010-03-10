#ifndef UAVOBJECTFIELD_H
#define UAVOBJECTFIELD_H

#include "uavobject.h"

class UAVObjectField: public QObject
{
    Q_OBJECT

public:
    /**
     * Recognized field types
     */
    typedef enum {
            FIELDTYPE_INT8 = 0,
            FIELDTYPE_INT16,
            FIELDTYPE_INT32,
            FIELDTYPE_FLOAT32,
            FIELDTYPE_CHAR
    } FieldType;

    UAVObjectField(QString& name, QString& units, FieldType type, quint32 numElements);
    void initialize(quint8* data, quint32 dataOffset, UAVObject* obj);
    UAVObject* getObject();
    QString getName();
    QString getUnits();
    FieldType getType();
    quint32 getNumElements();
    double getValue();
    void setValue(double value);
    double getValue(quint32 index);
    void setValue(double value, quint32 index);
    QString getString();
    void setString(QString& str);
    quint32 getDataOffset();
    quint32 getNumBytes();
    quint32 getNumBytesElement();

signals:
    void fieldUpdated(UAVObjectField* field);

private:
    QString name;
    QString units;
    FieldType type;
    quint32 numElements;
    quint32 numBytesPerElement;
    quint32 offset;
    quint8* data;
    UAVObject* obj;

    void clear();


};

#endif // UAVOBJECTFIELD_H
