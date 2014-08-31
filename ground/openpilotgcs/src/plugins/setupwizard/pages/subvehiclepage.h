/**
 ******************************************************************************
 *
 * @file       subvehiclepage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup
 * @{
 * @addtogroup SubVehiclePage
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

#ifndef SUBVEHICLEPAGEPAGE_H
#define SUBVEHICLEPAGEPAGE_H

#include <QtSvg/QGraphicsSvgItem>
#include <QtSvg/QSvgRenderer>
#include <QList>

#include "abstractwizardpage.h"

namespace Ui {
class SubVehiclePage;
}

class SubVehiclePage : public AbstractWizardPage {
    Q_OBJECT

public:
    explicit SubVehiclePage(SetupWizard *wizard, QWidget *parent = 0);
    ~SubVehiclePage();

    void initializePage();
    bool validatePage();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    Ui::SubVehiclePage *ui;
    void setupMultiTypesCombo();
    QSvgRenderer* m_renderer;
    QGraphicsSvgItem *m_multiPic;
    void updateAvailableTypes();
    QList<QString> m_descriptions;

private slots:
    void updateImageAndDescription();
};

#endif // SUBVEHICLEPAGEPAGE_H
