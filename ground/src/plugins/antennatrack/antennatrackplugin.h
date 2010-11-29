#ifndef ANTENNATRACKPLUGIN_H
#define ANTENNATRACKPLUGIN_H

#include <extensionsystem/iplugin.h>

class AntennaTrackGadgetFactory;

class AntennaTrackPlugin : public ExtensionSystem::IPlugin
{
public:
    AntennaTrackPlugin();
    ~AntennaTrackPlugin();

    void extensionsInitialized();
    bool initialize(const QStringList & arguments, QString * errorString);
    void shutdown();
private:
    AntennaTrackGadgetFactory *mf;
};

#endif // ANTENNATRACKPLUGIN_H
