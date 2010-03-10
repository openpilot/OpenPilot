#include "uavobjectsplugin.h"
#include "uavobjectsinit.h"

UAVObjectsPlugin::UAVObjectsPlugin()
{

}

UAVObjectsPlugin::~UAVObjectsPlugin()
{

}

void UAVObjectsPlugin::extensionsInitialized()
{

}

bool UAVObjectsPlugin::initialize(const QStringList & arguments, QString * errorString)
{
    // Create object manager and expose object
    UAVObjectManager* objMngr = new UAVObjectManager();
    addObject(objMngr);
    // Initialize UAVObjects
    UAVObjectsInitialize(objMngr);
    // Done
    Q_UNUSED(arguments);
    Q_UNUSED(errorString);
    return true;
}

void UAVObjectsPlugin::shutdown()
{

}
