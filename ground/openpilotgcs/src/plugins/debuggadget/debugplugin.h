/**
 ******************************************************************************
 *
 * @file       debugplugin.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup DebugGadgetPlugin Debug Gadget Plugin
 * @{
 * @brief A place holder gadget plugin 
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef DEBUGPLUGIN_H_
#define DEBUGPLUGIN_H_

#include <extensionsystem/iplugin.h>

class DebugGadgetFactory;

class DebugPlugin : public ExtensionSystem::IPlugin
{
public:
        DebugPlugin();
   ~DebugPlugin();

   void extensionsInitialized();
   bool initialize(const QStringList & arguments, QString * errorString);
   void shutdown();
private:
   DebugGadgetFactory *mf;
};
#endif /* DEBUGPLUGIN_H_ */
