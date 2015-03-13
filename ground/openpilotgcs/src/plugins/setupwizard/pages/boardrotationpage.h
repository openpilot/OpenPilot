/**
 ******************************************************************************
 *
 * @file       boardrotationpage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup
 * @{
 * @addtogroup BoardRotationPage
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

#ifndef BOARDROTATIONPAGE_H
#define BOARDROTATIONPAGE_H

#include <QtSvg>
#include "abstractwizardpage.h"
#include "boardrotation3dview.h"

namespace Ui {
class BoardRotationPage;
}

class BoardRotationPage : public AbstractWizardPage {
    Q_OBJECT

public:
    explicit BoardRotationPage(SetupWizard *wizard, QWidget *parent = 0);
    ~BoardRotationPage();
    bool validatePage();

private:
    Ui::BoardRotationPage *ui;
    // QGraphicsSvgItem *m_vehicleItem;
    BoardRotation3DView *m_board3DView;
    int m_prevRoll;
    int m_prevPitch;
    int m_prevYaw;

private slots:
    void onRollChanged();
    void onPitchChanged();
    void onYawChanged();
};

#endif // BOARDROTATIONPAGE_H
