/**
 ******************************************************************************
 *
 * @file       videogadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup VideoGadgetPlugin Video Gadget Plugin
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
<<<<<<< HEAD
#include "videogadgetconfiguration.h"
#include "videogadgetwidget.h"
#include "pipeline.h"

#include <QtCore>
#include <QDebug>
#include <QStringList>
#include <QTextEdit>
#include <QPushButton>
#include <QWidget>
=======
#include <QtCore>
#include <QDebug>
#include <QStringList>
#include <QtGui/QTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

#include "videogadgetconfiguration.h"
#include "videogadgetwidget.h"
#include "pipeline.h"
>>>>>>> 9bc03ffca82b2c41658cce9bf644bb735d193c28

VideoGadgetWidget::VideoGadgetWidget(QWidget *parent) :
        QFrame(parent)
{
    m_config = new Ui_Form();
    m_config->setupUi(this);

    m_config->textBrowser->hide();

    connect(m_config->video, SIGNAL(stateChanged(Pipeline::State)), this, SLOT(onStateChanged(Pipeline::State)));

    connect(m_config->startButton, SIGNAL(clicked()), m_config->video, SLOT(start()));
    connect(m_config->pauseButton, SIGNAL(clicked()), m_config->video, SLOT(pause()));
    connect(m_config->stopButton, SIGNAL(clicked()), m_config->video, SLOT(stop()));
}

void VideoGadgetWidget::onStateChanged(Pipeline::State newState)
{
//	msg(QString("start"));
//	m_config->video->start();
    switch (newState) {
    case Pipeline::Paused:
        m_config->startButton->setVisible(true);
        m_config->startButton->setEnabled(true);
        m_config->pauseButton->setVisible(false);
        m_config->pauseButton->setEnabled(false);
        m_config->stopButton->setEnabled(true);
        break;
    case Pipeline::Playing:
        m_config->startButton->setVisible(false);
        m_config->startButton->setEnabled(false);
        m_config->pauseButton->setVisible(true);
        m_config->pauseButton->setEnabled(true);
        m_config->stopButton->setEnabled(true);
        break;
    default:
        m_config->startButton->setVisible(true);
        m_config->startButton->setEnabled(true);
        m_config->pauseButton->setVisible(false);
        m_config->pauseButton->setEnabled(false);
        m_config->stopButton->setEnabled(false);
        break;
    }
}

VideoGadgetWidget::~VideoGadgetWidget()
{
    m_config = 0;
}

void VideoGadgetWidget::setConfiguration(VideoGadgetConfiguration *config)
{
    msg(QString("setting configuration"));
    m_config->video->setVisible(config->m_displayVideo);
    //m_config->control->setEnabled(config->m_displayControls);
    bool b = m_config->video->isPlaying();
    if (b) {
        stop();
    }
    m_config->video->setPipelineDesc(config->m_pipelineDesc);
    if (b || config->m_autoStart) {
        start();
    }
}

void VideoGadgetWidget::start()
{
    msg(QString("start"));
    m_config->startButton->setEnabled(false);
    m_config->video->start();
}

void VideoGadgetWidget::pause()
{
    msg(QString("pause"));
    m_config->pauseButton->setEnabled(false);
    m_config->video->pause();
}

void VideoGadgetWidget::stop()
{
    msg(QString("stop"));
//	m_config->startButton->setEnabled(false);
//	m_config->pauseButton->setEnabled(false);
//	m_config->stopButton->setEnabled(false);
    m_config->video->stop();
}

void VideoGadgetWidget::msg(const QString &str)
{
    qDebug() << str;
    if (m_config) {
        m_config->textBrowser->append(str);
    }
}

