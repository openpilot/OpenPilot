/**
 ******************************************************************************
 *
 * @file       autoupdatepage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup AutoUpdatePage
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

#ifndef AUTOUPDATEPAGE_H
#define AUTOUPDATEPAGE_H

#include <coreplugin/icore.h>
#include <coreplugin/connectionmanager.h>
#include "setupwizard.h"
#include "uavtalk/telemetrymanager.h"
#include "abstractwizardpage.h"
#include "uploader/enums.h"

namespace Ui {
class AutoUpdatePage;
}

class AutoUpdatePage : public AbstractWizardPage
{
    Q_OBJECT

public:
    explicit AutoUpdatePage(SetupWizard *wizard, QWidget *parent = 0);
    ~AutoUpdatePage();

private slots:
    void updateStatus(uploader::AutoUpdateStep ,QVariant);
    void disableButtons(){ enableButtons(false); }
    void enableButtons(bool enable);

private:
    Ui::AutoUpdatePage *ui;

};

#endif // AUTOUPDATEPAGE_H
