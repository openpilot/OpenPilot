/**
 ******************************************************************************
 *
 * @file       levellingpage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup LevellingPage
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

#ifndef LEVELLINGPAGE_H
#define LEVELLINGPAGE_H

#include "abstractwizardpage.h"
#include "levellingutil.h"

namespace Ui {
class LevellingPage;
}

class LevellingPage : public AbstractWizardPage
{
    Q_OBJECT

public:
    explicit LevellingPage(SetupWizard *wizard, QWidget *parent = 0);
    ~LevellingPage();
    bool validatePage();
    bool isComplete() const;

private slots:
    void performLevelling();
    void levellingProgress(long current, long total);
    void levellingDone(accelGyroBias bias);
    void levellingTimeout(QString message);

private:
    static const int BIAS_CYCLES = 200;
    static const int BIAS_RATE = 30;

    Ui::LevellingPage *ui;
    LevellingUtil *m_levellingUtil;

    void stopLevelling();
    void enableButtons(bool enable);
};

#endif // LEVELLINGPAGE_H
