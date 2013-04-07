/**
 ******************************************************************************
 *
 * @file       gcscontrolgadgetoptionspage.cpp
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

#include "gcscontrolgadgetoptionspage.h"
#include "gcscontrolgadgetconfiguration.h"
#include "ui_gcscontrolgadgetoptionspage.h"

#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>

GCSControlGadgetOptionsPage::GCSControlGadgetOptionsPage(GCSControlGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
    options_page = NULL;

   sdlGamepad = dynamic_cast<GCSControlPlugin*>(parent)->sdlGamepad;



}

GCSControlGadgetOptionsPage::~GCSControlGadgetOptionsPage()
{

}


void GCSControlGadgetOptionsPage::buttonState(ButtonNumber number, bool pressed)
{
    if (options_page) {
        QList<QCheckBox*> rbList;
        rbList << options_page->buttonInput0 <<
                  options_page->buttonInput1 << options_page->buttonInput2 <<
                  options_page->buttonInput3 << options_page->buttonInput4 <<
                  options_page->buttonInput5 << options_page->buttonInput6 <<
                  options_page->buttonInput7;

        if (number<8) // We only support 8 buttons
        {
            rbList.at(number)->setChecked(pressed);
        }
    }

}

void GCSControlGadgetOptionsPage::gamepads(quint8 count)
{
    Q_UNUSED(count);

    /*options_page->AvailableControllerList->clear();
    for (int i=0;i<count;i++)
    {
       options_page->AvailableControllerList->addItem(QString().sprintf("%d",i));//SDL_JoystickName(i));
    }*/

}

void GCSControlGadgetOptionsPage::axesValues(QListInt16 values)
{
    if (options_page) {
        QList<QProgressBar*> pbList;
        pbList << options_page->joyCh0 <<
                  options_page->joyCh1 << options_page->joyCh2 <<
                  options_page->joyCh3 << options_page->joyCh4 <<
                  options_page->joyCh5 << options_page->joyCh6 <<
                  options_page->joyCh7;
        int i=0;
        foreach (qint16 value, values) {
            if (i>7) break; // We only support 7 channels
            if (chRevList.at(i)->isChecked()==1)value = 65535 - value;
             if (pbList.at(i)->minimum() > value)
                 pbList.at(i)->setMinimum(value);
             if (pbList.at(i)->maximum() < value)
                 pbList.at(i)->setMaximum(value);
            pbList.at(i++)->setValue(value);
        }
    }
}


//creates options page widget (uses the UI file)
QWidget *GCSControlGadgetOptionsPage::createPage(QWidget *parent)
{
    Q_UNUSED(parent);

    int i;
    options_page = new Ui::GCSControlGadgetOptionsPage();
    QWidget *optionsPageWidget = new QWidget;
    options_page->setupUi(optionsPageWidget);



    //QList<QComboBox*> chList;
    chList.clear();
    chList << options_page->channel0 << options_page->channel1 <<
              options_page->channel2 << options_page->channel3 <<
              options_page->channel4 << options_page->channel5 <<
              options_page->channel6 << options_page->channel7;
    QStringList chOptions;
    chOptions << "None" << "Roll" << "Pitch" << "Yaw" << "Throttle";
    foreach (QComboBox* qb, chList) {
        qb->addItems(chOptions);
    }
    //QList<QCheckBox*> chRevList;
    chRevList.clear();
    chRevList << options_page->revCheckBox_1 << options_page->revCheckBox_2 <<
                 options_page->revCheckBox_3 << options_page->revCheckBox_4 <<
                 options_page->revCheckBox_5 << options_page->revCheckBox_6 <<
                 options_page->revCheckBox_7 << options_page->revCheckBox_8;

    //QList<QComboBox*> buttonFunctionList;
    buttonFunctionList.clear();
    buttonFunctionList << options_page->buttonFunction0 << options_page->buttonFunction1 <<
              options_page->buttonFunction2 << options_page->buttonFunction3 <<
              options_page->buttonFunction4 << options_page->buttonFunction5 <<
              options_page->buttonFunction6 << options_page->buttonFunction7;
    QStringList buttonOptions;
    buttonOptions <<"-" << "Roll" << "Pitch" << "Yaw" << "Throttle" << "Armed" << "GCS Control"; //added UDP control to action list
    foreach (QComboBox* qb, buttonFunctionList) {
        qb->addItems(buttonOptions);
    }
    //QList<QComboBox*> buttonActionList;
    buttonActionList.clear();
    buttonActionList << options_page->buttonAction0 << options_page->buttonAction1 <<
              options_page->buttonAction2 << options_page->buttonAction3 <<
              options_page->buttonAction4 << options_page->buttonAction5 <<
              options_page->buttonAction6 << options_page->buttonAction7;
    QStringList buttonActionOptions;
    buttonActionOptions << "Does nothing" << "Increases" << "Decreases" << "Toggles";
    foreach (QComboBox* qb, buttonActionList) {
        qb->addItems(buttonActionOptions);
    }
    //QList<QDoubleSpinBox*> buttonValueList;
    buttonValueList.clear();
    buttonValueList << options_page->buttonAmount0 << options_page->buttonAmount1 <<
              options_page->buttonAmount2 << options_page->buttonAmount3 <<
              options_page->buttonAmount4 << options_page->buttonAmount5 <<
              options_page->buttonAmount6 << options_page->buttonAmount7;
    //QList<QLabel*> buttonLabelList;
    buttonLabelList.clear();
    buttonLabelList << options_page->buttonLabel0 << options_page->buttonLabel1 <<
              options_page->buttonLabel2 << options_page->buttonLabel3 <<
              options_page->buttonLabel4 << options_page->buttonLabel5 <<
              options_page->buttonLabel6 << options_page->buttonLabel7;

    for (i=0;i<8;i++)
    {
        buttonActionList.at(i)->setCurrentIndex(m_config->getbuttonSettings(i).ActionID);
        buttonFunctionList.at(i)->setCurrentIndex(m_config->getbuttonSettings(i).FunctionID);
        buttonValueList.at(i)->setValue(m_config->getbuttonSettings(i).Amount);

        connect(buttonFunctionList.at(i),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonFunction()));
        //connect(buttonActionList.at(i),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonActions[i]()));
        updateButtonAction(i);
        buttonFunctionList.at(i)->setCurrentIndex(m_config->getbuttonSettings(i).FunctionID);
    }
    connect(buttonActionList.at(0),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonAction_0()));
    connect(buttonActionList.at(1),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonAction_1()));
    connect(buttonActionList.at(2),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonAction_2()));
    connect(buttonActionList.at(3),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonAction_3()));
    connect(buttonActionList.at(4),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonAction_4()));
    connect(buttonActionList.at(5),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonAction_5()));
    connect(buttonActionList.at(6),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonAction_6()));
    connect(buttonActionList.at(7),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonAction_7()));

    //updateButtonFunction();

    options_page->udp_host->setText(m_config->getUDPControlHost().toString());
    options_page->udp_port->setText(QString::number(m_config->getUDPControlPort()));


    // Controls mode are from 1 to 4.
    if (m_config->getControlsMode()>0 && m_config->getControlsMode() < 5)
        options_page->controlsMode->setCurrentIndex(m_config->getControlsMode()-1);
    else
        qDebug() << "GCSControl: Invalid control modes setting! Did you edit by hand?";

    QList<int> ql = m_config->getChannelsMapping();
    for (int i=0; i<4; i++) {
        if (ql.at(i) > -1)
            chList.at(ql.at(i))->setCurrentIndex(i+1);
    }
    QList<bool> qlChRev = m_config->getChannelsReverse();
    for (i=0; i<8; i++)
    {
        chRevList.at(i)->setChecked(qlChRev.at(i));;
    }

    connect(sdlGamepad,SIGNAL(axesValues(QListInt16)),this,SLOT(axesValues(QListInt16)));
    connect(sdlGamepad,SIGNAL(buttonState(ButtonNumber,bool)),this,SLOT(buttonState(ButtonNumber,bool)));
    connect(sdlGamepad,SIGNAL(gamepads(quint8)),this,SLOT(gamepads(quint8)));

    return optionsPageWidget;
}

/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void GCSControlGadgetOptionsPage::apply()
{
   m_config->setControlsMode(options_page->controlsMode->currentIndex()+1);
   /*QList<QComboBox*> chList;
   chList << options_page->channel0 << options_page->channel1 <<
             options_page->channel2 << options_page->channel3 <<
             options_page->channel4 << options_page->channel5 <<
             options_page->channel6 << options_page->channel7;
   QList<QComboBox*> buttonFunctionList;
   buttonFunctionList << options_page->buttonFunction0 << options_page->buttonFunction1 <<
             options_page->buttonFunction2 << options_page->buttonFunction3 <<
             options_page->buttonFunction4 << options_page->buttonFunction5 <<
             options_page->buttonFunction6 << options_page->buttonFunction7;
   QList<QComboBox*> buttonActionList;
   buttonActionList << options_page->buttonAction0 << options_page->buttonAction1 <<
             options_page->buttonAction2 << options_page->buttonAction3 <<
             options_page->buttonAction4 << options_page->buttonAction5 <<
             options_page->buttonAction6 << options_page->buttonAction7;
   QList<QDoubleSpinBox*> buttonValueList;
   buttonValueList << options_page->buttonAmount0 << options_page->buttonAmount1 <<
             options_page->buttonAmount2 << options_page->buttonAmount3 <<
             options_page->buttonAmount4 << options_page->buttonAmount5 <<
             options_page->buttonAmount6 << options_page->buttonAmount7;
*/

   int roll=-1 , pitch=-1, yaw=-1, throttle=-1;
   for (int i=0; i<chList.length(); i++) {
      switch (chList.at(i)->currentIndex()) {
      case 1:
          roll = i;
          break;
      case 2:
          pitch =i;
         break;
      case 3:
          yaw = i;
          break;
      case 4:
          throttle = i;
          break;
      }
   }
   m_config->setRPYTchannels(roll,pitch,yaw,throttle);

   m_config->setUDPControlSettings(options_page->udp_port->text().toInt(),options_page->udp_host->text());


   int j;
   for (j=0;j<8;j++)
   {
       m_config->setbuttonSettingsAction(j,buttonActionList.at(j)->currentIndex());
       m_config->setbuttonSettingsFunction(j,buttonFunctionList.at(j)->currentIndex());
       m_config->setbuttonSettingsAmount(j,buttonValueList.at(j)->value());
       m_config->setChannelReverse(j,chRevList.at(j)->isChecked());
   }


}

void GCSControlGadgetOptionsPage::finish()
{
    disconnect(sdlGamepad,0,this,0);
    delete options_page;
    options_page = NULL;
}


void GCSControlGadgetOptionsPage::updateButtonFunction()
{
    int i;
    /*QList<QComboBox*> buttonFunctionList;
    buttonFunctionList << options_page->buttonFunction0 << options_page->buttonFunction1 <<
              options_page->buttonFunction2 << options_page->buttonFunction3 <<
              options_page->buttonFunction4 << options_page->buttonFunction5 <<
              options_page->buttonFunction6 << options_page->buttonFunction7;
    QList<QComboBox*> buttonActionList;
    buttonActionList << options_page->buttonAction0 << options_page->buttonAction1 <<
              options_page->buttonAction2 << options_page->buttonAction3 <<
              options_page->buttonAction4 << options_page->buttonAction5 <<
              options_page->buttonAction6 << options_page->buttonAction7;
    QList<QDoubleSpinBox*> buttonValueList;
    buttonValueList << options_page->buttonAmount0 << options_page->buttonAmount1 <<
              options_page->buttonAmount2 << options_page->buttonAmount3 <<
              options_page->buttonAmount4 << options_page->buttonAmount5 <<
              options_page->buttonAmount6 << options_page->buttonAmount7;
    QList<QLabel*> buttonLabelList;
    buttonLabelList << options_page->buttonLabel0 << options_page->buttonLabel1 <<
              options_page->buttonLabel2 << options_page->buttonLabel3 <<
              options_page->buttonLabel4 << options_page->buttonLabel5 <<
              options_page->buttonLabel6 << options_page->buttonLabel7;
*/
    for (i=0;i<8;i++)
    {
        if (buttonActionList.at(i)->currentText().compare("Does nothing")==0)
        {
            buttonFunctionList.at(i)->setVisible(0);
            buttonLabelList.at(i)->setVisible(0);
            buttonValueList.at(i)->setVisible(0);
        }
        else
        if (buttonActionList.at(i)->currentText().compare("Toggles")==0)
        {
            buttonFunctionList.at(i)->setVisible(1);
            buttonLabelList.at(i)->setVisible(0);
            buttonValueList.at(i)->setVisible(0);
       }
        else
        {
            buttonFunctionList.at(i)->setVisible(1);
            buttonLabelList.at(i)->setVisible(1);
            buttonValueList.at(i)->setVisible(1);
        }
    }


}

void GCSControlGadgetOptionsPage::updateButtonAction(int controlID)
{
    int i;
    QStringList buttonOptions;
    /*QList<QComboBox*> buttonFunctionList;
    buttonFunctionList << options_page->buttonFunction0 << options_page->buttonFunction1 <<
              options_page->buttonFunction2 << options_page->buttonFunction3 <<
              options_page->buttonFunction4 << options_page->buttonFunction5 <<
              options_page->buttonFunction6 << options_page->buttonFunction7;
    QList<QComboBox*> buttonActionList;
    buttonActionList << options_page->buttonAction0 << options_page->buttonAction1 <<
              options_page->buttonAction2 << options_page->buttonAction3 <<
              options_page->buttonAction4 << options_page->buttonAction5 <<
              options_page->buttonAction6 << options_page->buttonAction7;
    QList<QDoubleSpinBox*> buttonValueList;
    buttonValueList << options_page->buttonAmount0 << options_page->buttonAmount1 <<
              options_page->buttonAmount2 << options_page->buttonAmount3 <<
              options_page->buttonAmount4 << options_page->buttonAmount5 <<
              options_page->buttonAmount6 << options_page->buttonAmount7;
    QList<QLabel*> buttonLabelList;
    buttonLabelList << options_page->buttonLabel0 << options_page->buttonLabel1 <<
              options_page->buttonLabel2 << options_page->buttonLabel3 <<
              options_page->buttonLabel4 << options_page->buttonLabel5 <<
              options_page->buttonLabel6 << options_page->buttonLabel7;
*/
    //for (i=0;i<8;i++)
    i=controlID;
    {
        if (buttonActionList.at(i)->currentText().compare("Does nothing")==0)
        {
            buttonFunctionList.at(i)->setVisible(0);
            buttonLabelList.at(i)->setVisible(0);
            buttonValueList.at(i)->setVisible(0);
        }
        else
        if (buttonActionList.at(i)->currentText().compare("Toggles")==0)
        {
            disconnect(buttonFunctionList.at(i),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonFunction()));
            buttonOptions <<"-" << "Armed" << "GCS Control" << "UDP Control";
            buttonFunctionList.at(i)->clear();
            buttonFunctionList.at(i)->insertItems(-1,buttonOptions);

            buttonFunctionList.at(i)->setVisible(1);
            buttonLabelList.at(i)->setVisible(0);
            buttonValueList.at(i)->setVisible(0);
            connect(buttonFunctionList.at(i),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonFunction()));
        }
        else
        {
            disconnect(buttonFunctionList.at(i),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonFunction()));
            buttonOptions <<"-" << "Roll" << "Pitch" << "Yaw" << "Throttle" ;
            buttonFunctionList.at(i)->clear();
            buttonFunctionList.at(i)->addItems(buttonOptions);

            buttonFunctionList.at(i)->setVisible(1);
            buttonLabelList.at(i)->setVisible(1);
            buttonValueList.at(i)->setVisible(1);
            connect(buttonFunctionList.at(i),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonFunction()));
        }
    }


}
