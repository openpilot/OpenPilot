/**
 ******************************************************************************
 *
 * @file       gcscontrolgadgetoptionspage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GCSControlGadgetPlugin GCSControl Gadget Plugin
 * @{
 * @brief A place holder gadget plugin
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

#ifndef GCSCONTROLGADGETOPTIONSPAGE_H
#define GCSCONTROLGADGETOPTIONSPAGE_H

#include "coreplugin/dialogs/ioptionspage.h"
#include "gcscontrolplugin.h"
#include "sdlgamepad/sdlgamepad.h"
#include <SDL/SDL.h>

#include <QDebug>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QLabel>

namespace Core {
class IUAVGadgetConfiguration;
}

class GCSControlGadgetConfiguration;

namespace Ui {
    class GCSControlGadgetOptionsPage;
}

using namespace Core;

class GCSControlGadgetOptionsPage : public IOptionsPage
{
Q_OBJECT
public:
    explicit GCSControlGadgetOptionsPage(GCSControlGadgetConfiguration *config, QObject *parent = 0);
    ~GCSControlGadgetOptionsPage();

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();

private:
    Ui::GCSControlGadgetOptionsPage *options_page;
    GCSControlGadgetConfiguration *m_config;
    SDLGamepad *sdlGamepad;

    QList<QComboBox*> chList;
    QList<QCheckBox*> chRevList;
    QList<QComboBox*> buttonFunctionList;
    QList<QComboBox*> buttonActionList;
    QList<QDoubleSpinBox*> buttonValueList;
    QList<QLabel*> buttonLabelList;

protected slots:
    // signals from joystick
    void gamepads(quint8 count);
    void buttonState(ButtonNumber number, bool pressed);
    void axesValues(QListInt16 values);
    void updateButtonFunction();
    void updateButtonAction(int controlID);
    void updateButtonAction_0(void){updateButtonAction(0);};
    void updateButtonAction_1(void){updateButtonAction(1);};
    void updateButtonAction_2(void){updateButtonAction(2);};
    void updateButtonAction_3(void){updateButtonAction(3);};
    void updateButtonAction_4(void){updateButtonAction(4);};
    void updateButtonAction_5(void){updateButtonAction(5);};
    void updateButtonAction_6(void){updateButtonAction(6);};
    void updateButtonAction_7(void){updateButtonAction(7);};
};

#endif // GCSCONTROLGADGETOPTIONSPAGE_H
