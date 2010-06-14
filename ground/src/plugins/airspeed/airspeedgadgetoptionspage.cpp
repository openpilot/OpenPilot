/**
 ******************************************************************************
 *
 * @file       airspeedgadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Airspeed Plugin Gadget options page
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   Airspeed
 * @{
 *
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

#include "airspeedgadgetoptionspage.h"
#include "airspeedgadgetconfiguration.h"
#include "ui_airspeedgadgetoptionspage.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavdataobject.h"


#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>

AirspeedGadgetOptionsPage::AirspeedGadgetOptionsPage(AirspeedGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
}

//creates options page widget (uses the UI file)
QWidget *AirspeedGadgetOptionsPage::createPage(QWidget *parent)
{

    options_page = new Ui::AirspeedGadgetOptionsPage();
    //main widget
    QWidget *optionsPageWidget = new QWidget;
    //main layout
    options_page->setupUi(optionsPageWidget);

    // Fills the combo boxes for the UAVObjects
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    QList< QList<UAVDataObject*> > objList = objManager->getDataObjects();
    foreach (QList<UAVDataObject*> list, objList) {
        foreach (UAVDataObject* obj, list) {
            options_page->uavObject1->addItem(obj->getName());
            options_page->uavObject2->addItem(obj->getName());
        }
    }

    // Fills the combo boxes for Needle movement options
    options_page->moveNeedle1->addItem("Rotate");
    options_page->moveNeedle1->addItem("Horizontal");
    options_page->moveNeedle1->addItem("Vertical");

    options_page->moveNeedle2->addItem("Rotate");
    options_page->moveNeedle2->addItem("Horizontal");
    options_page->moveNeedle2->addItem("Vertical");

    // Restore the contents from the settings:
    options_page->svgSourceFile->setText(m_config->dialFile());
    options_page->backgroundID->setText(m_config->dialBackground());
    options_page->foregroundID->setText(m_config->dialForeground());
    options_page->needle1ID->setText(m_config->dialNeedle1());
    options_page->needle2ID->setText(m_config->dialNeedle2());
    options_page->needle1Min->setValue(m_config->getN1Min());
    options_page->needle1Max->setValue(m_config->getN1Max());
    options_page->needle2Min->setValue(m_config->getN2Min());
    options_page->needle2Max->setValue(m_config->getN2Max());
    options_page->factor1->setValue(m_config->getN1Factor());
    options_page->factor2->setValue(m_config->getN2Factor());
    options_page->moveNeedle1->setCurrentIndex(options_page->moveNeedle1->findText(m_config->getN1Move()));
    options_page->moveNeedle2->setCurrentIndex(options_page->moveNeedle2->findText(m_config->getN2Move()));

    //select saved UAV Object field values
    if(options_page->uavObject1->findText(m_config->getN1DataObject())!=-1){
        options_page->uavObject1->setCurrentIndex(options_page->uavObject1->findText(m_config->getN1DataObject()));
        // Now load the object field values:
        UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(m_config->getN1DataObject()) );
        if (obj != NULL ) {
                QList<UAVObjectField*> fieldList = obj->getFields();                
                foreach (UAVObjectField* field, fieldList) {
                   options_page->objectField1->addItem(field->getName());
                }
                // And set the highlighed value from the settings:
                options_page->objectField1->setCurrentIndex(options_page->objectField1->findText(m_config->getN1ObjField()));
        }
    }
    connect(options_page->uavObject1, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_uavObject1_currentIndexChanged(QString)));
    if(options_page->uavObject2->findText(m_config->getN2DataObject())!=-1){
        options_page->uavObject2->setCurrentIndex(options_page->uavObject2->findText(m_config->getN2DataObject()));
        // Now load the object field values:
        UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(m_config->getN2DataObject()));
        if (obj != NULL ) {
                QList<UAVObjectField*> fieldList = obj->getFields();
                foreach (UAVObjectField* field, fieldList) {
                    options_page->objectField2->addItem(field->getName());
                }
                options_page->objectField2->setCurrentIndex(options_page->objectField2->findText(m_config->getN2ObjField()));
        }
    }
    connect(options_page->uavObject2, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_uavObject2_currentIndexChanged(QString)));
    connect(options_page->loadFile, SIGNAL(clicked()), this, SLOT(on_loadFile_clicked()));
    return optionsPageWidget;
}

/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void AirspeedGadgetOptionsPage::apply()
{
    m_config->setDialFile(options_page->svgSourceFile->text());
    m_config->setDialBackgroundID(options_page->backgroundID->text());
    m_config->setDialForegroundID(options_page->foregroundID->text());
    m_config->setDialNeedleID1(options_page->needle1ID->text());
    m_config->setDialNeedleID2(options_page->needle2ID->text());
    m_config->setN1Min(options_page->needle1Min->value());
    m_config->setN1Max(options_page->needle1Max->value());
    m_config->setN1Factor(options_page->factor1->value());
    m_config->setN2Min(options_page->needle2Min->value());
    m_config->setN2Max(options_page->needle2Max->value());
    m_config->setN2Factor(options_page->factor2->value());
    m_config->setN1DataObject(options_page->uavObject1->currentText());
    m_config->setN2DataObject(options_page->uavObject2->currentText());
    m_config->setN1ObjField(options_page->objectField1->currentText());
    m_config->setN2ObjField(options_page->objectField2->currentText());
    m_config->setN1Move(options_page->moveNeedle1->currentText());
    m_config->setN2Move(options_page->moveNeedle2->currentText());
}

/*
  Fills in the field1 combo box when value is changed in the
  object1 field
*/
void AirspeedGadgetOptionsPage::on_uavObject1_currentIndexChanged(QString val) {
    options_page->objectField1->clear();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(val) );
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        options_page->objectField1->addItem(field->getName());
    }
}

/*
  Fills in the field2 combo box when value is changed in the
  object1 field
*/
void AirspeedGadgetOptionsPage::on_uavObject2_currentIndexChanged(QString val) {
    options_page->objectField2->clear();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(val) );
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        options_page->objectField2->addItem(field->getName());
    }
}


/*
Opens an open file dialog.

*/
void AirspeedGadgetOptionsPage::on_loadFile_clicked()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(qobject_cast<QWidget*>(this),
                                                    tr("QFileDialog::getOpenFileName()"),
                                                    options_page->svgSourceFile->text(),
                                                    tr("All Files (*);;SVG Files (*.svg)"),
                                                    &selectedFilter,
                                                    options);
    if (!fileName.isEmpty()) options_page->svgSourceFile->setText(fileName);

}


void AirspeedGadgetOptionsPage::finish()
{

}
