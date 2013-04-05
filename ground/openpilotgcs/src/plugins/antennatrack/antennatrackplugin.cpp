#include "antennatrackplugin.h"
#include "antennatrackgadgetfactory.h"
#include <QDebug>
#include <QtPlugin>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>

AntennaTrackPlugin::AntennaTrackPlugin() {
    // Do nothing
 }

AntennaTrackPlugin::~AntennaTrackPlugin() {
    // Do nothing
}

bool AntennaTrackPlugin::initialize(const QStringList& args, QString *errMsg) {
    Q_UNUSED(args);
    Q_UNUSED(errMsg);

    mf = new AntennaTrackGadgetFactory(this);
    addAutoReleasedObject(mf);

    return true;
}

void AntennaTrackPlugin::extensionsInitialized() {
    // Do nothing
}

void AntennaTrackPlugin::shutdown() {
    // Do nothing
}

Q_EXPORT_PLUGIN(AntennaTrackPlugin)
