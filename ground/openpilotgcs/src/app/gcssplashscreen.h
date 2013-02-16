/**
 ******************************************************************************
 *
 * @file       gcssplashscreen.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup [Group]
 * @{
 * @addtogroup GCSSplashScreen
 * @{
 * @brief [Brief]
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

#ifndef GCSSPLASHSCREEN_H
#define GCSSPLASHSCREEN_H

#include <QSplashScreen>
#include <QPixmap>
#include <QPainter>
#include <extensionsystem/pluginspec.h>

#include "../../../../build/ground/openpilotgcs/gcsversioninfo.h"

class GCSSplashScreen : public QSplashScreen
{
    Q_OBJECT
public:
    explicit GCSSplashScreen();
    ~GCSSplashScreen();
    
public slots:
    void showPluginLoadingProgress(ExtensionSystem::PluginSpec *pluginSpec);
    void showProgressMessage(const QString &message) { drawMessageText(message); }

private:
    QPixmap *m_pixmap;
    QPainter *m_painter;
    void drawMessageText(const QString &message);

};

#endif // GCSSPLASHSCREEN_H
