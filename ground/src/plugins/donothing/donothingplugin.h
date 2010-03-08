#ifndef DONOTHINGPLUGIN_H 
#define DONOTHINGPLUGIN_H 

#include <extensionsystem/iplugin.h> 

class DoNothingPlugin : public ExtensionSystem::IPlugin 
{ 
public: 
   DoNothingPlugin(); 
   ~DoNothingPlugin(); 

   void extensionsInitialized(); 
   bool initialize(const QStringList & arguments, QString * errorString); 
   void shutdown(); 
}; 

#endif // DONOTHINGPLUGIN_H