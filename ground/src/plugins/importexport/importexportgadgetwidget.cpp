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
#include "xmlconfig.h"
#include "coreplugin/uavgadgetinstancemanager.h"
#include "coreplugin/icore.h"
#include <QtDebug>
#include <QSettings>
#include <QMessageBox>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>

ImportExportGadgetWidget::ImportExportGadgetWidget(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ImportExportGadgetWidget)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    ui->setupUi(this);
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
void ImportExportGadgetWidget::loadConfiguration(const ImportExportGadgetConfiguration* config)
{
    if ( !config )
        return;

    ui->configFile->setText(config->getIniFile());
}

void ImportExportGadgetWidget::on_exportButton_clicked()
{
    QString file = ui->configFile->text();
    qDebug() << "Export pressed! Write to file " << QFileInfo(file).absoluteFilePath();

    if ( QFileInfo(file).exists() ){
        QMessageBox msgBox;
        msgBox.setText(tr("File already exists."));
        msgBox.setInformativeText(tr("Do you want to overwrite the existing file?"));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);
        if ( msgBox.exec() == QMessageBox::Ok ){
            QFileInfo(file).absoluteDir().remove(QFileInfo(file).fileName());
        }
        else{
            qDebug() << "Export canceled!";
            return;
        }
    }
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


void ImportExportGadgetWidget::exportConfiguration(const QString& fileName)
{
    bool general = ui->checkBoxGeneral->isChecked();
    bool allGadgets = ui->checkBoxAllGadgets->isChecked();
    QSettings::Format format;
    if ( ui->radioButtonIniFormat->isChecked() ){
        format = QSettings::IniFormat;
    }
    else if ( ui->radioButtonXmlFormat->isChecked() ){
        format = XmlConfig::XmlSettingsFormat;
    }
    else {
        qWarning() << "Program Error in ImportExportGadgetWidget::exportConfiguration: unknown format. Assume XML!";
        format = XmlConfig::XmlSettingsFormat;
    }

    QSettings qs(fileName, format);

    if (general) {
        Core::ICore::instance()->saveMainSettings(&qs);
    }
    if (allGadgets) {
        Core::ICore::instance()->uavGadgetInstanceManager()->writeConfigurations(&qs);
    }
    qDebug() << "Export ended";
}


void ImportExportGadgetWidget::writeError(const QString& msg) const
{
    qWarning() << "ERROR: " << msg;
}

void ImportExportGadgetWidget::on_importButton_clicked()
{
    QString file = ui->configFile->text();
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
    bool general = ui->checkBoxGeneral->isChecked();
    bool allGadgets = ui->checkBoxAllGadgets->isChecked();
    QSettings::Format format;
    if ( ui->radioButtonIniFormat->isChecked() ){
        format = QSettings::IniFormat;
    }
    else if ( ui->radioButtonXmlFormat->isChecked() ){
        format = XmlConfig::XmlSettingsFormat;
    }
    else {
        qWarning() << "Program Error in ImportExportGadgetWidget::exportConfiguration: unknown format. Assume XML!";
        format = XmlConfig::XmlSettingsFormat;
    }

    QSettings qs(fileName, format);

    if (allGadgets) {
        Core::ICore::instance()->uavGadgetInstanceManager()->readConfigurations(&qs);
    }
    if (general) {
        Core::ICore::instance()->readMainSettings(&qs);
    }

    qDebug() << "Import ended";
}

void ImportExportGadgetWidget::on_helpButton_clicked()
{
    qDebug() << "Show Help";
    QDesktopServices::openUrl(QUrl("http://wiki.openpilot.org/Import_Export_plugin"));
}
/**
 * @}
 * @}
 */
