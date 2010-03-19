/*
 * emptyplugin.h
 *
 *  Created on: Mar 6, 2010
 *      Author: peter
 */

#ifndef EMPTYPLUGIN_H_
#define EMPTYPLUGIN_H_

#include <extensionsystem/iplugin.h>

class EmptyGadgetFactory;

class EmptyPlugin : public ExtensionSystem::IPlugin
{
public:
        EmptyPlugin();
   ~EmptyPlugin();

   void extensionsInitialized();
   bool initialize(const QStringList & arguments, QString * errorString);
   void shutdown();
private:
   EmptyGadgetFactory *mf;
};
#endif /* EMPTYPLUGIN_H_ */
