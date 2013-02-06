/**
 ******************************************************************************
 *
 * @file       importexportgadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @brief      Widget for Import/Export Plugin
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup   importexportplugin
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
#include "importexportgadgetwidget.h"
#include "ui_importexportgadgetwidget.h"
#include "utils/xmlconfig.h"
#include "coreplugin/uavgadgetinstancemanager.h"
#include "coreplugin/icore.h"
#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginspec.h>
#include <QtDebug>
#include <QSettings>
#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>

ImportExportGadgetWidget::ImportExportGadgetWidget(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ImportExportGadgetWidget)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    ui->setupUi(this);

	filename = "";
}

ImportExportGadgetWidget::~ImportExportGadgetWidget()
{
    delete ui;
}

void ImportExportGadgetWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ImportExportGadgetWidget::on_exportButton_clicked()
{
	QString file = filename;
	QString filter = tr("GCS Settings file (*.xml)");
	file = QFileDialog::getSaveFileName(this, tr("Save GCS Settings too file .."), QFileInfo(file).absoluteFilePath(), filter).trimmed();
	if (file.isEmpty()) {
		return;
	}

	// Add a "XML" extension to the file in case it does not exist:
	if (!file.toLower().endsWith(".xml"))
        file.append(".xml");

	filename = file;

    qDebug() << "Export pressed! Write to file " << QFileInfo(file).absoluteFilePath();

    QMessageBox msgBox;
    QDir dir = QFileInfo(file).absoluteDir();
    if (! dir.exists()) {
        msgBox.setText(tr("Can't write file ") + QFileInfo(file).absoluteFilePath()
                       + " since directory "+ dir.absolutePath() + " doesn't exist!");
        msgBox.exec();
        return;
    }
    exportConfiguration(file);

    msgBox.setText(tr("The settings have been exported to ") + QFileInfo(file).absoluteFilePath());
    msgBox.exec();
    emit done();

}

QList<Core::IConfigurablePlugin*> ImportExportGadgetWidget::getConfigurables()
{
    QList<Core::IConfigurablePlugin*> configurables;

    QList<ExtensionSystem::PluginSpec *> specs = ExtensionSystem::PluginManager::instance()->plugins();
    foreach ( ExtensionSystem::PluginSpec* spec, specs ){
        if ( Core::IConfigurablePlugin* plugin = dynamic_cast<Core::IConfigurablePlugin*>(spec->plugin()) ){
            qDebug()<< "Configurable: " << plugin->metaObject()->className();
            configurables.append(plugin);
        }
    }
    return configurables;
}

void ImportExportGadgetWidget::exportConfiguration(const QString& fileName)
{
    bool doGeneral = ui->checkBoxGeneral->isChecked();
    bool doAllGadgets = ui->checkBoxAllGadgets->isChecked();
    bool doPlugins = ui->checkBoxPlugins->isChecked();

    QSettings::Format format = XmlConfig::XmlSettingsFormat;
    QSettings qs(fileName, format);

    if (doGeneral) {
        Core::ICore::instance()->saveMainSettings(&qs);
    }
    if (doAllGadgets) {
        Core::ICore::instance()->uavGadgetInstanceManager()->saveSettings(&qs);
    }
    if ( doPlugins ){
        foreach ( Core::IConfigurablePlugin *plugin, getConfigurables()){
            Core::ICore::instance()->saveSettings(plugin,&qs);
        }
    }

    qDebug() << "Export ended";
}


void ImportExportGadgetWidget::writeError(const QString& msg) const
{
    qWarning() << "ERROR: " << msg;
}

void ImportExportGadgetWidget::on_importButton_clicked()
{
	QString file = filename;
	QString filter = tr("GCS Settings file (*.xml)");
	file = QFileDialog::getOpenFileName(this, tr("Load GCS Settings from file .."), QFileInfo(file).absoluteFilePath(), filter).trimmed();
	if (file.isEmpty()) {
		return;
	}

	filename = file;

    qDebug() << "Import pressed! Read from file " << QFileInfo(file).absoluteFilePath();

    QMessageBox msgBox;
    if (! QFileInfo(file).isReadable()) {
        msgBox.setText(tr("Can't read file ") + QFileInfo(file).absoluteFilePath());
        msgBox.exec();
        return;
    }
    importConfiguration(file);

    // The new configs are added to the old configs. Things are messy now.
    msgBox.setText(tr("The settings have been imported from ") + QFileInfo(file).absoluteFilePath()
                   + tr(". Restart the application."));
    msgBox.exec();
    emit done();
}

void ImportExportGadgetWidget::importConfiguration(const QString& fileName)
{
    bool doGeneral = ui->checkBoxGeneral->isChecked();
    bool doAllGadgets = ui->checkBoxAllGadgets->isChecked();
    bool doPlugins = ui->checkBoxPlugins->isChecked();

    QSettings qs(fileName, XmlConfig::XmlSettingsFormat);

    if ( doAllGadgets ) {
        Core::ICore::instance()->uavGadgetInstanceManager()->readSettings(&qs);
    }
    if ( doGeneral ) {
        Core::ICore::instance()->readMainSettings(&qs);
    }
    if ( doPlugins ){
        foreach ( Core::IConfigurablePlugin *plugin, getConfigurables()){
            Core::ICore::instance()->readSettings(plugin,&qs);
        }
    }

    qDebug() << "Import ended";
}

void ImportExportGadgetWidget::on_helpButton_clicked()
{
    QDesktopServices::openUrl(QUrl("http://wiki.openpilot.org/x/OQBj"));
}


void ImportExportGadgetWidget::on_resetButton_clicked()
{
    QMessageBox msgBox;
    msgBox.setText(tr("All your settings will be deleted!"));
    msgBox.setInformativeText(tr("You must restart the GCS in order to activate the changes."));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    if ( msgBox.exec() == QMessageBox::Ok ){
        qDebug() << "Reset requested!";
        Core::ICore::instance()->deleteSettings();
    }
    else{
        qDebug() << "Reset canceled!";
        return;
    }
}


/**
 * @}
 * @}
 */

