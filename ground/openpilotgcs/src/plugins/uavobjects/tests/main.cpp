#include <QtCore/QCoreApplication>
#include <QTextStream>
#include "uavobjectstest.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    UAVObjectsTest* test = new UAVObjectsTest();

    return a.exec();
}
