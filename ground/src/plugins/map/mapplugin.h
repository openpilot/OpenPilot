/*
 * mapplugin.h
 *
 *  Created on: Mar 6, 2010
 *      Author: peter
 */

#ifndef MAPPLUGIN_H_
#define MAPPLUGIN_H_

#include <extensionsystem/iplugin.h>

class MapGadgetFactory;

class MapPlugin : public ExtensionSystem::IPlugin
{
public:
	MapPlugin();
   ~MapPlugin();

   void extensionsInitialized();
   bool initialize(const QStringList & arguments, QString * errorString);
   void shutdown();
private:
   MapGadgetFactory *mf;
};
#endif /* MAPPLUGIN_H_ */
