/**
 ******************************************************************************
 *
 * @file       scopegadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ScopePlugin Scope Gadget Plugin
 * @{
 * @brief The scope Gadget, graphically plots the states of UAVObjects
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

#include "scopegadgetoptionspage.h"

#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavdataobject.h"


#include <QtGui/qpalette.h>


ScopeGadgetOptionsPage::ScopeGadgetOptionsPage(ScopeGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
    //nothing to do here...
}

//creates options page widget (uses the UI file)
QWidget* ScopeGadgetOptionsPage::createPage(QWidget *parent)
{
    Q_UNUSED(parent);

    options_page = new Ui::ScopeGadgetOptionsPage();
    //main widget
    QWidget *optionsPageWidget = new QWidget;
    //main layout
    options_page->setupUi(optionsPageWidget);

    options_page->cmbPlotType->addItem("Sequencial Plot","");
    options_page->cmbPlotType->addItem("Chronological Plot","");

    // Fills the combo boxes for the UAVObjects
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    QList< QList<UAVDataObject*> > objList = objManager->getDataObjects();
    foreach (QList<UAVDataObject*> list, objList) {
        foreach (UAVDataObject* obj, list) {
            options_page->cmbUAVObjects->addItem(obj->getName());
        }
    }

    //Connect signals to slots cmbUAVObjects.currentIndexChanged
    connect(options_page->cmbUAVObjects, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_cmbUAVObjects_currentIndexChanged(QString)));

    if(options_page->cmbUAVObjects->currentIndex() >= 0)
        on_cmbUAVObjects_currentIndexChanged(options_page->cmbUAVObjects->currentText());

    options_page->cmbScale->addItem("E-9", -9);
    options_page->cmbScale->addItem("E-6", -6);
    options_page->cmbScale->addItem("E-5",-5);
    options_page->cmbScale->addItem("E-4",-4);
    options_page->cmbScale->addItem("E-3",-3);
    options_page->cmbScale->addItem("E-2",-2);
    options_page->cmbScale->addItem("E-1",-1);
    options_page->cmbScale->addItem("E0",0);
    options_page->cmbScale->addItem("E1",1);
    options_page->cmbScale->addItem("E2",2);
    options_page->cmbScale->addItem("E3",3);
    options_page->cmbScale->addItem("E4",4);
    options_page->cmbScale->addItem("E5",5);
    options_page->cmbScale->addItem("E6",6);
    options_page->cmbScale->addItem("E9",9);
    options_page->cmbScale->addItem("E12",12);
    options_page->cmbScale->setCurrentIndex(7);

    //Set widget values from settings
    options_page->cmbPlotType->setCurrentIndex(m_config->plotType());
    options_page->spnDataSize->setValue(m_config->dataSize());
    options_page->spnRefreshInterval->setValue(m_config->refreshInterval());

    //add the configured curves
    foreach (PlotCurveConfiguration* plotData,  m_config->plotCurveConfigs()) {

        QString uavObject = plotData->uavObject;
        QString uavField = plotData->uavField;
        int scale = plotData->yScalePower;
        QVariant varColor = plotData->color;

        addPlotCurveConfig(uavObject,uavField,scale,varColor);
    }

    if(m_config->plotCurveConfigs().count() > 0)
        options_page->lstCurves->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);

    connect(options_page->btnAddCurve, SIGNAL(clicked()), this, SLOT(on_btnAddCurve_clicked()));
    connect(options_page->btnRemoveCurve, SIGNAL(clicked()), this, SLOT(on_btnRemoveCurve_clicked()));
    connect(options_page->lstCurves, SIGNAL(currentRowChanged(int)), this, SLOT(on_lstCurves_currentRowChanged(int)));
    connect(options_page->btnColor, SIGNAL(clicked()), this, SLOT(on_btnColor_clicked()));

    setYAxisWidgetFromPlotCurve();

    //logging path setup
    options_page->LoggingPath->setExpectedKind(Utils::PathChooser::Directory);
    options_page->LoggingPath->setPromptDialogTitle(tr("Choose Logging Directory"));
    options_page->LoggingPath->setPath(m_config->getLoggingPath());
    options_page->LoggingConnect->setChecked(m_config->getLoggingNewFileOnConnect());
    options_page->LoggingEnable->setChecked(m_config->getLoggingEnabled());
    connect(options_page->LoggingEnable, SIGNAL(clicked()), this, SLOT(on_loggingEnable_clicked()));
    on_loggingEnable_clicked();



    return optionsPageWidget;
}

void ScopeGadgetOptionsPage::on_btnColor_clicked()
 {
     QColor color = QColorDialog::getColor( QColor(options_page->btnColor->text()), options_page->widget);
     if (color.isValid()) {
         setButtonColor(color);
     }
 }

/*!
  \brief Populate the widgets that containts the configs for the Y-Axis from
  the selected plot curve
  */
void ScopeGadgetOptionsPage::setYAxisWidgetFromPlotCurve()
{
    bool parseOK = false;
    QListWidgetItem* listItem = options_page->lstCurves->currentItem();

    if(listItem == 0)
        return;

    int currentIndex = options_page->cmbUAVObjects->findText( listItem->data(Qt::UserRole + 0).toString());
    options_page->cmbUAVObjects->setCurrentIndex(currentIndex);

    currentIndex = options_page->cmbUAVField->findText( listItem->data(Qt::UserRole + 1).toString());
    options_page->cmbUAVField->setCurrentIndex(currentIndex);

    currentIndex = options_page->cmbScale->findData( listItem->data(Qt::UserRole + 2), Qt::UserRole, Qt::MatchExactly);
    options_page->cmbScale->setCurrentIndex(currentIndex);

    QVariant varColor  = listItem->data(Qt::UserRole + 3);
    int rgb = varColor.toInt(&parseOK);

    setButtonColor(QColor((QRgb)rgb));
}

void ScopeGadgetOptionsPage::setButtonColor(const QColor &color)
{
    options_page->btnColor->setAutoFillBackground(true);
    options_page->btnColor->setText(color.name());
    options_page->btnColor->setPalette(QPalette(color));
}

/*!
  \brief When a new UAVObject is selected, populate the UAVObject field combo box with the correct values.
  */
void ScopeGadgetOptionsPage::on_cmbUAVObjects_currentIndexChanged(QString val)
{
    options_page->cmbUAVField->clear();

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(val) );

    if (obj == NULL)
        return; // Rare case: the config contained a UAVObject name which does not exist anymore.

    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        if(field->getType() == UAVObjectField::STRING || field->getType() == UAVObjectField::ENUM )
            continue;

        if(field->getElementNames().count() > 1)
        {
            foreach(QString elemName , field->getElementNames())
            {
                options_page->cmbUAVField->addItem(field->getName() + "-" + elemName);
            }
        }
        else
            options_page->cmbUAVField->addItem(field->getName());
    }
}

/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void ScopeGadgetOptionsPage::apply()
{
    bool parseOK = false;

    //Apply configuration changes
    m_config->setPlotType(options_page->cmbPlotType->currentIndex());
    m_config->setDataSize(options_page->spnDataSize->value());
    m_config->setRefreashInterval(options_page->spnRefreshInterval->value());

    QList<PlotCurveConfiguration*> plotCurveConfigs;
    for(int iIndex = 0; iIndex < options_page->lstCurves->count();iIndex++) {
        QListWidgetItem* listItem = options_page->lstCurves->item(iIndex);

        PlotCurveConfiguration* newPlotCurveConfigs = new PlotCurveConfiguration();
        newPlotCurveConfigs->uavObject = listItem->data(Qt::UserRole + 0).toString();
        newPlotCurveConfigs->uavField  = listItem->data(Qt::UserRole + 1).toString();
        newPlotCurveConfigs->yScalePower  = listItem->data(Qt::UserRole + 2).toInt(&parseOK);
        if(!parseOK)
            newPlotCurveConfigs->yScalePower = 0;

        QVariant varColor  = listItem->data(Qt::UserRole + 3);
        int rgb = varColor.toInt(&parseOK);
        if(!parseOK)
            newPlotCurveConfigs->color = QColor(Qt::black).rgb();
        else
            newPlotCurveConfigs->color = (QRgb)rgb;

        plotCurveConfigs.append(newPlotCurveConfigs);
    }

    m_config->replacePlotCurveConfig(plotCurveConfigs);

    //save the logging config
    m_config->setLoggingPath(options_page->LoggingPath->path());
    m_config->setLoggingNewFileOnConnect(options_page->LoggingConnect->isChecked());
    m_config->setLoggingEnabled(options_page->LoggingEnable->isChecked());

}

/*!
  \brief Add a new curve to the plot.
*/
void ScopeGadgetOptionsPage::on_btnAddCurve_clicked()
{
    bool parseOK = false;
    QString uavObject = options_page->cmbUAVObjects->currentText();
    QString uavField = options_page->cmbUAVField->currentText();
    int scale = options_page->cmbScale->itemData(options_page->cmbScale->currentIndex()).toInt(&parseOK);

    if(!parseOK)
       scale = 0;


    QVariant varColor = (int)QColor(options_page->btnColor->text()).rgb();

    //Find an existing plot curve config based on the uavobject and uav field. If it
    //exists, update it, else add a new one.
    if(options_page->lstCurves->count() &&
       options_page->lstCurves->currentItem()->text() == uavObject + "." + uavField)
    {
        QListWidgetItem *listWidgetItem = options_page->lstCurves->currentItem();
        setCurvePlotProperties(listWidgetItem,uavObject,uavField,scale,varColor);
    }else
    {
        addPlotCurveConfig(uavObject,uavField,scale,varColor);

        options_page->lstCurves->setCurrentRow(options_page->lstCurves->count() - 1);
    }
}

void ScopeGadgetOptionsPage::addPlotCurveConfig(QString uavObject, QString uavField, int scale, QVariant varColor)
{
    //Add a new curve config to the list
    QString listItemDisplayText = uavObject + "." + uavField;
    options_page->lstCurves->addItem(listItemDisplayText);
    QListWidgetItem *listWidgetItem = options_page->lstCurves->item(options_page->lstCurves->count() - 1);

    setCurvePlotProperties(listWidgetItem,uavObject,uavField,scale,varColor);

}

void ScopeGadgetOptionsPage::setCurvePlotProperties(QListWidgetItem *listWidgetItem,QString uavObject, QString uavField, int scale, QVariant varColor)
{
    bool parseOK = false;

    //Set the properties of the newly added list item
    QString listItemDisplayText = uavObject + "." + uavField;
    QRgb rgbColor = (QRgb)varColor.toInt(&parseOK);
    QColor color = QColor( rgbColor );
    //listWidgetItem->setText(listItemDisplayText);
    listWidgetItem->setTextColor( color );

    //Store some additional data for the plot curve on the list item
    listWidgetItem->setData(Qt::UserRole + 0,QVariant(uavObject));
    listWidgetItem->setData(Qt::UserRole + 1,QVariant(uavField));
    listWidgetItem->setData(Qt::UserRole + 2,QVariant(scale));
    listWidgetItem->setData(Qt::UserRole + 3,varColor);
}

/*!
  Remove a curve config from the plot.
  */
void ScopeGadgetOptionsPage::on_btnRemoveCurve_clicked()
{
    options_page->lstCurves->takeItem(options_page->lstCurves->currentIndex().row());
}

void ScopeGadgetOptionsPage::finish()
{

}

/*!
  When a different plot curve config is selected, populate its values into the widgets.
  */
void ScopeGadgetOptionsPage::on_lstCurves_currentRowChanged(int currentRow)
{
    Q_UNUSED(currentRow);
    setYAxisWidgetFromPlotCurve();
}

void ScopeGadgetOptionsPage::on_loggingEnable_clicked()
 {
    bool en = options_page->LoggingEnable->isChecked();
    options_page->LoggingPath->setEnabled(en);
    options_page->LoggingConnect->setEnabled(en);
    options_page->LoggingLabel->setEnabled(en);

 }
