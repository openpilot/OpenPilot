/**
 ******************************************************************************
 *
 * @file       systemhealthplugin.h
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup SystemHealthPlugin System Health Plugin
 * @{
 * @brief The System Health gadget plugin
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

#ifndef SYSTEMHEALTHPLUGIN_H_
#define SYSTEMHEALTHPLUGIN_H_

#include <extensionsystem/iplugin.h>

class SystemHealthGadgetFactory;

class SystemHealthPlugin : public ExtensionSystem::IPlugin
{
public:
        SystemHealthPlugin();
        ~SystemHealthPlugin();

   void extensionsInitialized();
   bool initialize(const QStringList & arguments, QString * errorString);
   void shutdown();
private:
   SystemHealthGadgetFactory *mf;
};
#endif /* SYSTEMHEALTHPLUGIN_H_ */
