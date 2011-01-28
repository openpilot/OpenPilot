/**
 ******************************************************************************
 *
 * @file       lineardialgadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup LinearDialPlugin Linear Dial Plugin
 * @{
 * @brief Impliments a gadget that displays linear gauges 
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
#include "uavobjectmanager.h"
#include "uavdataobject.h"

#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>
#include <QFontDialog>

LineardialGadgetOptionsPage::LineardialGadgetOptionsPage(LineardialGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
    font = QFont("Arial", 12); // Default in case nothing exists yet.
}

//creates options page widget (uses the UI file)
QWidget *LineardialGadgetOptionsPage::createPage(QWidget *parent)
{

    Q_UNUSED(parent);
    options_page = new Ui::LineardialGadgetOptionsPage();
    //main widget
    QWidget *optionsPageWidget = new QWidget;
    //main layout
    options_page->setupUi(optionsPageWidget);


    // Restore the contents from the settings:
    options_page->svgSourceFile->setExpectedKind(Utils::PathChooser::File);
    options_page->svgSourceFile->setPromptDialogFilter(tr("SVG image (*.svg)"));
    options_page->svgSourceFile->setPromptDialogTitle(tr("Choose SVG image"));
    options_page->svgSourceFile->setPath(m_config->getDialFile());
    options_page->minValue->setValue(m_config->getMin());
    options_page->maxValue->setValue(m_config->getMax());
    // Do this by hand (in case value is zero, no signal would
    // be sent!
    on_rangeMin_valueChanged(m_config->getMin());
    on_rangeMax_valueChanged(m_config->getMax());
    options_page->greenMin->setValue(m_config->getGreenMin());
    options_page->greenMax->setValue(m_config->getGreenMax());
    options_page->yellowMin->setValue(m_config->getYellowMin());
    options_page->yellowMax->setValue(m_config->getYellowMax());
    options_page->redMin->setValue(m_config->getRedMin());
    options_page->redMax->setValue(m_config->getRedMax());
    options_page->factor->setValue(m_config->getFactor());
    options_page->decPlaces->setValue(m_config->getDecimalPlaces());
	font.fromString(m_config->getFont());
	options_page->useOpenGL->setChecked(m_config->useOpenGL());

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
                on_objectName_currentIndexChanged(m_config->getSourceDataObject());
                // And set the highlighed value from the settings:
                options_page->objectField->setCurrentIndex(options_page->objectField->findText(m_config->getSourceObjectField()));
        }
    }

    connect(options_page->objectName, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_objectName_currentIndexChanged(QString)));
    connect(options_page->fontPicker, SIGNAL(clicked()), this, SLOT(on_fontPicker_clicked()));
    connect(options_page->minValue, SIGNAL(valueChanged(double)), this, SLOT(on_rangeMin_valueChanged(double)));
    connect(options_page->maxValue, SIGNAL(valueChanged(double)), this, SLOT(on_rangeMax_valueChanged(double)));

    return optionsPageWidget;
}

/**
 * Used to make sure the green/yellow/red ranges are consistent
 * with the overall dial range
 */
void LineardialGadgetOptionsPage::on_rangeMin_valueChanged(double val)
{
    options_page->maxValue->setMinimum(val);

    options_page->greenMin->setMinimum(val);
    options_page->yellowMin->setMinimum(val);
    options_page->redMin->setMinimum(val);

    options_page->greenMax->setMinimum(val);
    options_page->yellowMax->setMinimum(val);
    options_page->redMax->setMinimum(val);

}

/**
 * Used to make sure the green/yellow/red ranges are consistent
 * with the overall dial range
 */
void LineardialGadgetOptionsPage::on_rangeMax_valueChanged(double val)
{
    options_page->minValue->setMaximum(val);


    options_page->greenMax->setMaximum(val);
    options_page->yellowMax->setMaximum(val);
    options_page->redMax->setMaximum(val);

    options_page->greenMin->setMaximum(val);
    options_page->yellowMin->setMaximum(val);
    options_page->redMin->setMaximum(val);
}


/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void LineardialGadgetOptionsPage::apply()
{
    m_config->setDialFile(options_page->svgSourceFile->path());
    double rangeMin = options_page->minValue->value();
    double rangeMax = options_page->maxValue->value();
    m_config->setRange(rangeMin,rangeMax);
    m_config->setGreenRange(options_page->greenMin->value(),options_page->greenMax->value());
    m_config->setYellowRange(options_page->yellowMin->value(),options_page->yellowMax->value());
    m_config->setRedRange(options_page->redMin->value(),options_page->redMax->value());
    m_config->setSourceDataObject(options_page->objectName->currentText());
    m_config->setSourceObjField(options_page->objectField->currentText());
    m_config->setFont(font.toString());
    m_config->setDecimalPlaces(options_page->decPlaces->value());
    m_config->setFactor(options_page->factor->value());
	m_config->setUseOpenGL(options_page->useOpenGL->checkState());
}

/**
 * Opens a font picker.
 *
 */
void LineardialGadgetOptionsPage::on_fontPicker_clicked()
{
    bool ok;
    font = QFontDialog::getFont(&ok, font , qobject_cast<QWidget*>(this));
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
        if(field->getElementNames().count() > 1)
        {
            foreach(QString elemName , field->getElementNames())
            {
                options_page->objectField->addItem(field->getName() + "-" + elemName);
            }
        }
        else
            options_page->objectField->addItem(field->getName());
    }
}

void LineardialGadgetOptionsPage::finish()
{

}
