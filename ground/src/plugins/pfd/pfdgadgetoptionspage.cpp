/**
 ******************************************************************************
 *
 * @file       pfdgadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Primary Flight Display Plugin Gadget options page
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   pfdplugin
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

#include "pfdgadgetoptionspage.h"
#include "pfdgadgetconfiguration.h"
#include "ui_pfdgadgetoptionspage.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavdataobject.h"


#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>

PFDGadgetOptionsPage::PFDGadgetOptionsPage(PFDGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
}

//creates options page widget (uses the UI file)
QWidget *PFDGadgetOptionsPage::createPage(QWidget *parent)
{

    options_page = new Ui::PFDGadgetOptionsPage();
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
            options_page->uavObject2->addItem(obj->getName());
            options_page->uavObject3->addItem(obj->getName());
        }
    }


    // Restore the contents from the settings:
    options_page->svgSourceFile->setText(m_config->dialFile());
    options_page->needle2Min->setValue(m_config->getN2Min());
    options_page->needle2Max->setValue(m_config->getN2Max());
    options_page->needle3Min->setValue(m_config->getN3Min());
    options_page->needle3Max->setValue(m_config->getN3Max());
    options_page->factor2->setValue(m_config->getN2Factor());
    options_page->factor3->setValue(m_config->getN3Factor());


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

    if(options_page->uavObject3->findText(m_config->getN3DataObject())!=-1){
        options_page->uavObject3->setCurrentIndex(options_page->uavObject3->findText(m_config->getN3DataObject()));
        // Now load the object field values:
        UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(m_config->getN3DataObject()));
        if (obj != NULL ) {
                QList<UAVObjectField*> fieldList = obj->getFields();
                foreach (UAVObjectField* field, fieldList) {
                    options_page->objectField3->addItem(field->getName());
                }
                options_page->objectField3->setCurrentIndex(options_page->objectField3->findText(m_config->getN3ObjField()));
        }
    }
    connect(options_page->uavObject3, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_uavObject3_currentIndexChanged(QString)));

    connect(options_page->loadFile, SIGNAL(clicked()), this, SLOT(on_loadFile_clicked()));
    return optionsPageWidget;
}

/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void PFDGadgetOptionsPage::apply()
{
    m_config->setDialFile(options_page->svgSourceFile->text());
    m_config->setN2Min(options_page->needle2Min->value());
    m_config->setN2Max(options_page->needle2Max->value());
    m_config->setN2Factor(options_page->factor2->value());
    m_config->setN3Min(options_page->needle3Min->value());
    m_config->setN3Max(options_page->needle3Max->value());
    m_config->setN3Factor(options_page->factor3->value());

    m_config->setN2DataObject(options_page->uavObject2->currentText());
    m_config->setN3DataObject(options_page->uavObject3->currentText());

    m_config->setN2ObjField(options_page->objectField2->currentText());
    m_config->setN3ObjField(options_page->objectField3->currentText());
}


/*
  Fills in the field2 combo box when value is changed in the
  object2 field
*/
void PFDGadgetOptionsPage::on_uavObject2_currentIndexChanged(QString val) {
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
  Fills in the field3 combo box when value is changed in the
  object3 field
*/
void PFDGadgetOptionsPage::on_uavObject3_currentIndexChanged(QString val) {
    options_page->objectField3->clear();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(val) );
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        options_page->objectField3->addItem(field->getName());
    }
}


/*
Opens an open file dialog.

*/
void PFDGadgetOptionsPage::on_loadFile_clicked()
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


void PFDGadgetOptionsPage::finish()
{
}
