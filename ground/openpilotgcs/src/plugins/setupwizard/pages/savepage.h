/**
 ******************************************************************************
 *
 * @file       savepage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup SavePage
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

#ifndef SAVEPAGE_H
#define SAVEPAGE_H

#include "abstractwizardpage.h"

namespace Ui {
class SavePage;
}

class SavePage : public AbstractWizardPage
{
    Q_OBJECT

public:
    explicit SavePage(SetupWizard *wizard, QWidget *parent = 0);
    ~SavePage();
    bool validatePage();
    bool isComplete() const;

private:
    Ui::SavePage *ui;
    bool m_successfulWrite;
    void enableButtons(bool enable);

private slots:
    void writeToController();
    void saveProgress(int total, int current, QString description);
};

#endif // SAVEPAGE_H
