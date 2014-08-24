/**
 ******************************************************************************
 *
 * @file       fixedwingpage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup FixedWingPage
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

#ifndef AIRSPEEDPAGE_H
#define AIRSPEEDPAGE_H

#include <QtSvg/QGraphicsSvgItem>
#include <QtSvg/QSvgRenderer>
#include <QList>

#include "abstractwizardpage.h"

namespace Ui {
class AirSpeedPage;
}

class AirSpeedPage : public AbstractWizardPage {
    Q_OBJECT

public:
    explicit AirSpeedPage(SetupWizard *wizard, QWidget *parent = 0);
    ~AirSpeedPage();

    void initializePage();
    bool validatePage();

    void fitInView();
protected:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);

private:
    Ui::AirSpeedPage *ui;
    void setupAirSpeedPageTypesCombo();
    QGraphicsSvgItem *m_fixedwingPic;
    void updateAvailableTypes();
    QList<QString> m_descriptions;

private slots:
    void updateImageAndDescription();

};

#endif // AIRSPEEDPAGE_H
