/**
 ******************************************************************************
 *
 * @file       notyetimplementedpage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup NotYetImplementedPage
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

#ifndef NOTYETIMPLEMENTEDPAGE_H
#define NOTYETIMPLEMENTEDPAGE_H

#include "abstractwizardpage.h"

namespace Ui {
class NotYetImplementedPage;
}

class NotYetImplementedPage : public AbstractWizardPage
{
    Q_OBJECT
    
public:
    explicit NotYetImplementedPage(SetupWizard *wizard, QWidget *parent = 0);
    ~NotYetImplementedPage();
    
private:
    Ui::NotYetImplementedPage *ui;
};

#endif // NOTYETIMPLEMENTEDPAGE_H
