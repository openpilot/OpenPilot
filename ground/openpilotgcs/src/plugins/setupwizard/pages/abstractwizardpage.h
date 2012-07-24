/**
 ******************************************************************************
 *
 * @file       abstractwizardpage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup AbstractWizardPage
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

#ifndef ABSTRACTWIZARDPAGE_H
#define ABSTRACTWIZARDPAGE_H

#include <QWizardPage>
#include "setupwizard.h"

class AbstractWizardPage : public QWizardPage
{
    Q_OBJECT
protected:
    explicit AbstractWizardPage(SetupWizard *wizard, QWidget *parent = 0);

private:
    SetupWizard *m_wizard;

public:
    SetupWizard* getWizard() { return m_wizard; }

};

#endif // ABSTRACTWIZARDPAGE_H
