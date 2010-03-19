/*
 * scopeplugin.h
 *
 *  Created on: Mar 6, 2010
 *      Author: peter
 */

#ifndef SCOPEPLUGIN_H_
#define SCOPEPLUGIN_H_

#include <extensionsystem/iplugin.h>

class ScopeGadgetFactory;

class ScopePlugin : public ExtensionSystem::IPlugin
{
public:
        ScopePlugin();
   ~ScopePlugin();

   void extensionsInitialized();
   bool initialize(const QStringList & arguments, QString * errorString);
   void shutdown();
private:
   ScopeGadgetFactory *mf;
};
#endif /* SCOPEPLUGIN_H_ */
