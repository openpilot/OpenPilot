/**
 ******************************************************************************
 *
 * @file       rebootpage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup RebootPage
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

#ifndef REBOOTPAGE_H
#define REBOOTPAGE_H

#include "abstractwizardpage.h"

namespace Ui {
class RebootPage;
}

class RebootPage : public AbstractWizardPage
{
    Q_OBJECT
    
public:
    explicit RebootPage(SetupWizard *wizard, QWidget *parent = 0);
    ~RebootPage();

    void initializePage();
    bool validatePage();
    
private:
    Ui::RebootPage *ui;
    QTimer m_timer;
    bool m_toggl;

private slots:
    void toggleLabel();
};

#endif // REBOOTPAGE_H
