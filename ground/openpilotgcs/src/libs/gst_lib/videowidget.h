/**
 ******************************************************************************
 *
 * @file       videogadgetwidget.h
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

#ifndef VIDEOWIDGET_H_
#define VIDEOWIDGET_H_

#include <QtGui/QWidget>
#include <QtGui/QResizeEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QMouseEvent>

#include "gst_global.h"
#include "pipeline.h"
#include "overlay.h"

typedef struct _GstElement GstElement;

class BusSyncHandler;

class GST_LIB_EXPORT VideoWidget: public QWidget {
Q_OBJECT
public:
    VideoWidget(QWidget *parent = 0);
    ~VideoWidget();
public:
    void setPipelineDesc(QString pipelineDesc);
    bool isPlaying();
public slots:
    void start();
    void pause();
    void stop();
//public slots:
//	void onMessage(GstMessage *message);
//signals:
//	void message(QString msg);
signals:
    void stateChanged(Pipeline::State newState);
protected:
    QString getStatus();
    QString getStatusMessage();
    void paintStatus(QPaintEvent *);
    // QWidget overrides
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
private:
    void init();
    void dispose();
    void setOverlay(Overlay *);
    // QWidget overrides
    bool event(QEvent *);
    QPaintEngine * paintEngine() const;
private:
    QString pipelineDesc;
    QString lastError;
    GstElement *pipeline;
    Overlay *overlay;
    BusSyncHandler *handler;
    bool reset;
};

#endif /* VIDEOWIDGET_H_ */
