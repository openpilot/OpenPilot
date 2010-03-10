#ifndef UAVOBJECTSPLUGIN_H
#define UAVOBJECTSPLUGIN_H

#include <extensionsystem/iplugin.h>
#include <QtPlugin>
#include "uavobjectmanager.h"

class UAVObjectsPlugin: public ExtensionSystem::IPlugin
{
public:
    UAVObjectsPlugin();
    ~UAVObjectsPlugin();

    void extensionsInitialized();
    bool initialize(const QStringList & arguments, QString * errorString);
    void shutdown();
};

#endif // UAVOBJECTSPLUGIN_H
