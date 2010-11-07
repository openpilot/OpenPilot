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

}

void GCSControlGadgetOptionsPage::gamepads(quint8 count)
{

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
    options_page = new Ui::GCSControlGadgetOptionsPage();
    QWidget *optionsPageWidget = new QWidget;
    options_page->setupUi(optionsPageWidget);

    QList<QComboBox*> chList;
    chList << options_page->channel0 << options_page->channel1 <<
              options_page->channel2 << options_page->channel3 <<
              options_page->channel4 << options_page->channel5 <<
              options_page->channel6 << options_page->channel7;
    QStringList chOptions;
    chOptions << "None" << "Roll" << "Pitch" << "Yaw" << "Throttle";
    foreach (QComboBox* qb, chList) {
        qb->addItems(chOptions);
    }

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

    connect(sdlGamepad,SIGNAL(axesValues(QListInt16)),this,SLOT(axesValues(QListInt16)));

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
   QList<QComboBox*> chList;
   chList << options_page->channel0 << options_page->channel1 <<
             options_page->channel2 << options_page->channel3 <<
             options_page->channel4 << options_page->channel5 <<
             options_page->channel6 << options_page->channel7;
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

}

void GCSControlGadgetOptionsPage::finish()
{
    disconnect(sdlGamepad,0,this,0);
    delete options_page;
    options_page = NULL;
}
