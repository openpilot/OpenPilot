/**
 ******************************************************************************
 *
 * @file       modelviewgadgetconfiguration.h
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

#ifndef MODELVIEWGADGETCONFIGURATION_H
#define MODELVIEWGADGETCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>

using namespace Core;

class ModelViewGadgetConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT
public:
    explicit ModelViewGadgetConfiguration(QString classId, QSettings* qSettings = 0, QObject *parent = 0);

    void saveConfig(QSettings* settings) const;
    IUAVGadgetConfiguration *clone();
    QString acFilename() {return m_acFilename;}
    void setAcFilename(QString acFile) { m_acFilename = acFile; }
    QString bgFilename() { return m_bgFilename; }
    void setBgFilename(QString bgFile) { m_bgFilename = bgFile; }
    bool vboEnabled() { return m_enableVbo; }
    void setVboEnabled(bool vboEnable) { m_enableVbo = vboEnable; }
signals:

public slots:

private:
    QString m_acFilename;
    QString m_bgFilename;
    bool m_enableVbo;  // Vertex buffer objects, a few GPUs crash if enabled
};

#endif // MODELVIEWGADGETCONFIGURATION_H
