/**
 ******************************************************************************
 *
 * @file       connectiondiagram.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup ConnectionDiagram
 * @{
 * @brief
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

#ifndef CONNECTIONDIAGRAM_H
#define CONNECTIONDIAGRAM_H

#include <QDialog>
#include <QHash>
#include <QSvgRenderer>

#include <QGraphicsSvgItem>
#include "vehicleconfigurationsource.h"


namespace Ui {
class ConnectionDiagram;
}

class ConnectionDiagram : public QDialog
{
    Q_OBJECT
    
public:
    explicit ConnectionDiagram(QWidget *parent, VehicleConfigurationSource *configSource);
    ~ConnectionDiagram();
    
private:
    Ui::ConnectionDiagram *ui;
    VehicleConfigurationSource *m_configSource;

    QSvgRenderer *m_renderer;
    QGraphicsSvgItem* m_background;
    QGraphicsScene *m_scene;

    void setupGraphicsScene();
    void setupGraphicsSceneItems(QList<QString> elementsToShow);
protected:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);

private slots:

    void on_saveButton_clicked();
};

#endif // CONNECTIONDIAGRAM_H
