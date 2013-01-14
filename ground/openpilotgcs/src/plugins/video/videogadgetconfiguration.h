/**
 ******************************************************************************
 *
 * @file       videogadgetconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup VideoPlugin Video Plugin
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

#ifndef VIDEOGADGETCONFIGURATION_H
#define VIDEOGADGETCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>

using namespace Core;

class VideoGadgetConfiguration: public IUAVGadgetConfiguration {
    Q_OBJECT
public:
    explicit VideoGadgetConfiguration(QString classId, QSettings *qSettings = 0, QObject *parent = 0);

    void saveConfig(QSettings *settings) const;
    IUAVGadgetConfiguration *clone();
    bool displayVideo()
    {
        return m_displayVideo;
    }
    void setDisplayVideo(bool displayVideo)
    {
        m_displayVideo = displayVideo;
    }
    bool displayControls()
    {
        return m_displayControls;
    }
    void setDisplayControls(bool displayControls)
    {
        m_displayControls = displayControls;
    }
    bool autoStart()
    {
        return m_autoStart;
    }
    void setAutoStart(bool autoStart)
    {
        m_autoStart = autoStart;
    }
    bool respectAspectRatio()
    {
        return m_respectAspectRatio;
    }
    void setRespectAspectRatio(bool respectAspectRatio)
    {
        m_respectAspectRatio = respectAspectRatio;
    }
    QString pipelineDesc()
    {
        return m_pipelineDesc;
    }
    void setPipelineDesc(QString pipelineDesc)
    {
        m_pipelineDesc = pipelineDesc;
    }
    QString pipelineInfo()
    {
        return m_pipelineInfo;
    }
    void setPipelineInfo(QString pipelineInfo)
    {
        m_pipelineInfo = pipelineInfo;
    }
//signals:

//public slots:

public:
    // video
    bool m_displayVideo;
    bool m_respectAspectRatio;
    // controls
    bool m_displayControls;
    bool m_autoStart;
    QString m_pipelineDesc;
    QString m_pipelineInfo;
};

#endif // VIDEOGADGETCONFIGURATION_H
