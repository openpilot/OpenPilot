/**
 ******************************************************************************
 *
 * @file       dialgadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup DialPlugin Dial Plugin
 * @{
 * @brief Plots flight information rotary style dials 
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

#include "dialgadgetoptionspage.h"
#include "dialgadgetconfiguration.h"
#include "ui_dialgadgetoptionspage.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavdataobject.h"


#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>

DialGadgetOptionsPage::DialGadgetOptionsPage(DialGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
}

//creates options page widget (uses the UI file)
QWidget *DialGadgetOptionsPage::createPage(QWidget *parent)
{

    Q_UNUSED(parent);
    options_page = new Ui::DialGadgetOptionsPage();
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
            options_page->uavObject3->addItem(obj->getName());
        }
    }

    // Fills the combo boxes for Needle movement options
    options_page->moveNeedle1->addItem("Rotate");
    options_page->moveNeedle1->addItem("Horizontal");
    options_page->moveNeedle1->addItem("Vertical");

    options_page->moveNeedle2->addItem("Rotate");
    options_page->moveNeedle2->addItem("Horizontal");
    options_page->moveNeedle2->addItem("Vertical");

    options_page->moveNeedle3->addItem("Rotate");
    options_page->moveNeedle3->addItem("Horizontal");
    options_page->moveNeedle3->addItem("Vertical");

    // Restore the contents from the settings:

    options_page->svgSourceFile->setExpectedKind(Utils::PathChooser::File);
    options_page->svgSourceFile->setPromptDialogFilter(tr("SVG image (*.svg)"));
    options_page->svgSourceFile->setPromptDialogTitle(tr("Choose SVG image"));
    options_page->svgSourceFile->setPath(m_config->dialFile());
    options_page->backgroundID->setText(m_config->dialBackground());
    options_page->foregroundID->setText(m_config->dialForeground());
    options_page->needle1ID->setText(m_config->dialNeedle1());
    options_page->needle2ID->setText(m_config->dialNeedle2());
    options_page->needle3ID->setText(m_config->dialNeedle3());
    options_page->needle1Min->setValue(m_config->getN1Min());
    options_page->needle1Max->setValue(m_config->getN1Max());
    options_page->needle2Min->setValue(m_config->getN2Min());
    options_page->needle2Max->setValue(m_config->getN2Max());
    options_page->needle3Min->setValue(m_config->getN3Min());
    options_page->needle3Max->setValue(m_config->getN3Max());
    options_page->factor1->setValue(m_config->getN1Factor());
    options_page->factor2->setValue(m_config->getN2Factor());
    options_page->factor3->setValue(m_config->getN3Factor());
    options_page->moveNeedle1->setCurrentIndex(options_page->moveNeedle1->findText(m_config->getN1Move()));
    options_page->moveNeedle2->setCurrentIndex(options_page->moveNeedle2->findText(m_config->getN2Move()));
    options_page->moveNeedle3->setCurrentIndex(options_page->moveNeedle3->findText(m_config->getN3Move()));

	options_page->useOpenGL->setChecked(m_config->useOpenGL());
	options_page->smoothUpdates->setChecked(m_config->getBeSmooth());


    //select saved UAV Object field values
    if(options_page->uavObject1->findText(m_config->getN1DataObject())!=-1){
        options_page->uavObject1->setCurrentIndex(options_page->uavObject1->findText(m_config->getN1DataObject()));
        // Now load the object field values - 1st check that the object saved in the config still exists
        UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(m_config->getN1DataObject()) );
        if (obj != NULL ) {
                on_uavObject1_currentIndexChanged(m_config->getN1DataObject());
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
            on_uavObject2_currentIndexChanged(m_config->getN2DataObject());
            options_page->objectField2->setCurrentIndex(options_page->objectField2->findText(m_config->getN2ObjField()));
        }
    }
    connect(options_page->uavObject2, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_uavObject2_currentIndexChanged(QString)));

    if(options_page->uavObject3->findText(m_config->getN3DataObject())!=-1){
        options_page->uavObject3->setCurrentIndex(options_page->uavObject3->findText(m_config->getN3DataObject()));
        // Now load the object field values:
        UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(m_config->getN3DataObject()));
        if (obj != NULL ) {
            on_uavObject3_currentIndexChanged(m_config->getN3DataObject());
                options_page->objectField3->setCurrentIndex(options_page->objectField3->findText(m_config->getN3ObjField()));
        }
    }
    connect(options_page->uavObject3, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_uavObject3_currentIndexChanged(QString)));

    connect(options_page->fontPicker, SIGNAL(clicked()), this, SLOT(on_fontPicker_clicked()));

    return optionsPageWidget;
}

/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void DialGadgetOptionsPage::apply()
{
    m_config->setDialFile(options_page->svgSourceFile->path());
	m_config->setDialBackgroundID(options_page->backgroundID->text());
    m_config->setDialForegroundID(options_page->foregroundID->text());
    m_config->setDialNeedleID1(options_page->needle1ID->text());
    m_config->setDialNeedleID2(options_page->needle2ID->text());
    m_config->setDialNeedleID3(options_page->needle3ID->text());
    m_config->setN1Min(options_page->needle1Min->value());
    m_config->setN1Max(options_page->needle1Max->value());
    m_config->setN1Factor(options_page->factor1->value());
    m_config->setN2Min(options_page->needle2Min->value());
    m_config->setN2Max(options_page->needle2Max->value());
    m_config->setN2Factor(options_page->factor2->value());
    m_config->setN3Min(options_page->needle3Min->value());
    m_config->setN3Max(options_page->needle3Max->value());
    m_config->setN3Factor(options_page->factor3->value());
    m_config->setN1DataObject(options_page->uavObject1->currentText());
    m_config->setN2DataObject(options_page->uavObject2->currentText());
    m_config->setN3DataObject(options_page->uavObject3->currentText());
    m_config->setN1ObjField(options_page->objectField1->currentText());
    m_config->setN2ObjField(options_page->objectField2->currentText());
    m_config->setN3ObjField(options_page->objectField3->currentText());
    m_config->setN1Move(options_page->moveNeedle1->currentText());
    m_config->setN2Move(options_page->moveNeedle2->currentText());
    m_config->setN3Move(options_page->moveNeedle3->currentText());
    m_config->setFont(font.toString());
	m_config->setUseOpenGL(options_page->useOpenGL->checkState());
	m_config->setBeSmooth(options_page->smoothUpdates->checkState());
}

/**
 * Opens a font picker.
 *
 */
void DialGadgetOptionsPage::on_fontPicker_clicked()
{
    bool ok;
     font = QFontDialog::getFont(&ok, QFont("Arial", 12), qobject_cast<QWidget*>(this));
}


/*
  Fills in the field1 combo box when value is changed in the
  object1 field
*/
void DialGadgetOptionsPage::on_uavObject1_currentIndexChanged(QString val) {
    options_page->objectField1->clear();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(val) );
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        if(field->getType() == UAVObjectField::STRING || field->getType() == UAVObjectField::ENUM )
            continue;
        if(field->getElementNames().count() > 1)
        {
            foreach(QString elemName , field->getElementNames())
            {
                options_page->objectField1->addItem(field->getName() + "-" + elemName);
            }
        }
        else
            options_page->objectField1->addItem(field->getName());
    }
}

/*
  Fills in the field2 combo box when value is changed in the
  object2 field
*/
void DialGadgetOptionsPage::on_uavObject2_currentIndexChanged(QString val) {
    options_page->objectField2->clear();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(val) );
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        if(field->getType() == UAVObjectField::STRING || field->getType() == UAVObjectField::ENUM )
            continue;
        if(field->getElementNames().count() > 1)
        {
            foreach(QString elemName , field->getElementNames())
            {
                options_page->objectField2->addItem(field->getName() + "-" + elemName);
            }
        }
        else
            options_page->objectField2->addItem(field->getName());
    }
}

/*
  Fills in the field3 combo box when value is changed in the
  object3 field
*/
void DialGadgetOptionsPage::on_uavObject3_currentIndexChanged(QString val) {
    options_page->objectField3->clear();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(val) );
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        if(field->getType() == UAVObjectField::STRING || field->getType() == UAVObjectField::ENUM )
            continue;
        if(field->getElementNames().count() > 1)
        {
            foreach(QString elemName , field->getElementNames())
            {
                options_page->objectField3->addItem(field->getName() + "-" + elemName);
            }
        }
        else
            options_page->objectField3->addItem(field->getName());
    }
}


void DialGadgetOptionsPage::finish()
{

}
