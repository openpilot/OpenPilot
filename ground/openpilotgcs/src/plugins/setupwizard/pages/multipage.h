/**
 ******************************************************************************
 *
 * @file       multipage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup MultiPage
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

#ifndef MULTIPAGE_H
#define MULTIPAGE_H

#include <QtSvg/QGraphicsSvgItem>
#include <QtSvg/QSvgRenderer>
#include <QList>

#include "abstractwizardpage.h"

namespace Ui {
class MultiPage;
}

class MultiPage : public AbstractWizardPage
{
    Q_OBJECT
    
public:
    explicit MultiPage(SetupWizard *wizard, QWidget *parent = 0);
    ~MultiPage();

    void initializePage();
    bool validatePage();

protected:
    void resizeEvent(QResizeEvent *event);
    
private:
    Ui::MultiPage *ui;
    void setupMultiTypesCombo();
    QGraphicsSvgItem *m_multiPic;
    void updateAvailableTypes();
    QList<QString> m_descriptions;

private slots:
    void updateImageAndDescription();
};

#endif // MULTIPAGE_H
