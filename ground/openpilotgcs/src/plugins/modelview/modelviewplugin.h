/**
 ******************************************************************************
 *
 * @file       modelviewplugin.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ModelViewPlugin ModelView Plugin
 * @{
 * @brief A gadget that displays a 3D representation of the UAV 
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

#ifndef MODELVIEWPLUGIN_H_
#define MODELVIEWPLUGIN_H_

#include <extensionsystem/iplugin.h>

class ModelViewGadgetFactory;

class ModelViewPlugin : public ExtensionSystem::IPlugin
{
public:
   ModelViewPlugin();
   ~ModelViewPlugin();

   void extensionsInitialized();
   bool initialize(const QStringList & arguments, QString * errorString);
   void shutdown();
private:
   ModelViewGadgetFactory *mvf;
};
#endif /* MODELVIEWPLUGIN_H_ */
