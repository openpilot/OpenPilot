/**
 ******************************************************************************
 *
 * @file       lineardialgadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Airspeed Plugin Gadget options page
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   lineardial
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

#include "lineardialgadgetoptionspage.h"
#include "lineardialgadgetconfiguration.h"
#include "ui_lineardialgadgetoptionspage.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavdataobject.h"

#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>

LineardialGadgetOptionsPage::LineardialGadgetOptionsPage(LineardialGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
}

//creates options page widget (uses the UI file)
QWidget *LineardialGadgetOptionsPage::createPage(QWidget *parent)
{

    options_page = new Ui::LineardialGadgetOptionsPage();
    //main widget
    QWidget *optionsPageWidget = new QWidget;
    //main layout
    options_page->setupUi(optionsPageWidget);

    // Restore the contents from the settings:
    options_page->svgSourceFile->setText(m_config->getDialFile());
    options_page->minValue->setValue(m_config->getMin());
    options_page->maxValue->setValue(m_config->getMax());
    options_page->greenMin->setValue(m_config->getGreenMin());
    options_page->greenMax->setValue(m_config->getGreenMax());
    options_page->yellowMin->setValue(m_config->getYellowMin());
    options_page->yellowMax->setValue(m_config->getYellowMax());
    options_page->redMin->setValue(m_config->getRedMin());
    options_page->redMax->setValue(m_config->getRedMax());

    // Fills the combo boxes for the UAVObjects
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    QList< QList<UAVDataObject*> > objList = objManager->getDataObjects();
    foreach (QList<UAVDataObject*> list, objList) {
        foreach (UAVDataObject* obj, list) {
            options_page->objectName->addItem(obj->getName());
        }
    }
    //select saved UAV Object field values
    if(options_page->objectName->findText(m_config->getSourceDataObject())!=-1){
        options_page->objectName->setCurrentIndex(options_page->objectName->findText(m_config->getSourceDataObject()));
        // Now load the object field values:
        UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(m_config->getSourceDataObject()) );
        if (obj != NULL ) {
                QList<UAVObjectField*> fieldList = obj->getFields();
                foreach (UAVObjectField* field, fieldList) {
                   options_page->objectField->addItem(field->getName());
                }
                // And set the highlighed value from the settings:
                options_page->objectField->setCurrentIndex(options_page->objectField->findText(m_config->getSourceObjectField()));
        }
    }

    connect(options_page->objectName, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_objectName_currentIndexChanged(QString)));
    connect(options_page->loadFile, SIGNAL(clicked()), this, SLOT(on_loadFile_clicked()));

    return optionsPageWidget;
}

/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void LineardialGadgetOptionsPage::apply()
{
    m_config->setDialFile(options_page->svgSourceFile->text());
    m_config->setRange(options_page->minValue->value(),options_page->maxValue->value());
    m_config->setGreenRange(options_page->greenMin->value(),options_page->greenMax->value());
    m_config->setYellowRange(options_page->yellowMin->value(),options_page->yellowMax->value());
    m_config->setRedRange(options_page->redMin->value(),options_page->redMax->value());
    m_config->setSourceDataObject(options_page->objectName->currentText());
    m_config->setSourceObjField(options_page->objectField->currentText());
}

/**

Opens an open file dialog.

*/
void LineardialGadgetOptionsPage::on_loadFile_clicked()
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

/*
  Fills in the field1 combo box when value is changed in the
  object1 field
*/
void LineardialGadgetOptionsPage::on_objectName_currentIndexChanged(QString val) {
    options_page->objectField->clear();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(val) );
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        options_page->objectField->addItem(field->getName());
    }
}

void LineardialGadgetOptionsPage::finish()
{

}
