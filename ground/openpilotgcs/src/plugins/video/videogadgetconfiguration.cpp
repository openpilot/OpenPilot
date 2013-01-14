/**
 ******************************************************************************
 *
 * @file       videogadgetconfiguration.cpp
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

#include "videogadgetconfiguration.h"

VideoGadgetConfiguration::VideoGadgetConfiguration(QString classId, QSettings* qSettings, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent),
    m_pipelineDesc(""), m_pipelineInfo("")
{
    // if a saved configuration exists load it
    if(qSettings != 0) {
        m_displayVideo = qSettings->value("displayVideo").toBool();
        m_autoStart = qSettings->value("autoStart").toBool();
        m_displayControls = qSettings->value("displayControls").toBool();
        m_respectAspectRatio = qSettings->value("respectAspectRatio").toBool();
        m_pipelineDesc = qSettings->value("pipelineDesc").toString();
        m_pipelineInfo = qSettings->value("pipelineInfo").toString();
     }
}

IUAVGadgetConfiguration *VideoGadgetConfiguration::clone()
{
    VideoGadgetConfiguration *mv = new VideoGadgetConfiguration(this->classId());
    mv->m_displayVideo = m_displayVideo;
    mv->m_autoStart = m_autoStart;
    mv->m_displayControls = m_displayControls;
    mv->m_respectAspectRatio = m_respectAspectRatio;
    mv->m_pipelineDesc = m_pipelineDesc;
    mv->m_pipelineInfo = m_pipelineInfo;
    return mv;
}

/**
 * Saves a configuration.
 *
 */
void VideoGadgetConfiguration::saveConfig(QSettings* qSettings) const {
	   qSettings->setValue("displayVideo", m_displayVideo);
	   qSettings->setValue("autoStart", m_autoStart);
	   qSettings->setValue("displayControls", m_displayControls);
	   qSettings->setValue("respectAspectRatio", m_respectAspectRatio);
	   qSettings->setValue("pipelineDesc", m_pipelineDesc);
	   qSettings->setValue("pipelineInfo", m_pipelineInfo);
}
