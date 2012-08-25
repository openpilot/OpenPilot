/**
 ******************************************************************************
 *
 * @file       plugin.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2012.
 * @addtogroup 3rdParty Third-party integration
 * @{
 * @addtogroup AeroSimRC AeroSimRC proxy plugin
 * @{
 * @brief AeroSimRC simulator to HITL proxy plugin
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

#ifndef PLUGIN_H
#define PLUGIN_H

#include <QtCore>
#include <QTime>
#include <QList>
#include "aerosimrcdatastruct.h"

#define SIM_DLL_EXPORT extern "C" __declspec(dllexport)

SIM_DLL_EXPORT void AeroSIMRC_Plugin_ReportStructSizes(
    quint32 *sizeSimToPlugin,
    quint32 *sizePluginToSim,
    quint32 *sizePluginInit
);

SIM_DLL_EXPORT void AeroSIMRC_Plugin_Init(
    pluginInit *p
);

SIM_DLL_EXPORT void AeroSIMRC_Plugin_Run(
    const simToPlugin *stp,
    pluginToSim *pts
);

#endif // PLUGIN_H
