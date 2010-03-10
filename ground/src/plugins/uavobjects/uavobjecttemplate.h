#ifndef $(NAMEUC)_H
#define $(NAMEUC)_H

#include "uavdataobject.h"

class $(NAME): UAVDataObject
{
public:     
    static const quint32 OBJID = $(OBJID);
    static const QString NAME = QString($(NAME));
    static const bool SINGLEINST = $(SINGLEINST);
    static const quint32 NUMBYTES = sizeof($(NAME)Data);

    typedef struct {
            $(DATAFIELDS)
    } __attribute__((packed)) $(NAME)Data;

    $(NAME)();

    $(NAME)Data getData();
    void setData($(NAME)Data& data);

private:
    $(NAME)Data data;

};

#endif // $(NAMEUC)_H
