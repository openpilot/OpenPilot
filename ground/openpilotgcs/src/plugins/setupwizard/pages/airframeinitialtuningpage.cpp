/**
 ******************************************************************************
 *
 * @file       airframestabfixedwingpage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup AirframeStabFixedwingPage
 * @{
 * @brief
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

#include "airframeinitialtuningpage.h"
#include "ui_airframeinitialtuningpage.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include "vehicletemplateexportdialog.h"

AirframeInitialTuningPage::AirframeInitialTuningPage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent),
    ui(new Ui::AirframeInitialTuningPage), m_dir(NULL), m_photoItem(NULL)
{
    ui->setupUi(this);
    ui->templateImage->setScene(new QGraphicsScene());
    connect(ui->templateList, SIGNAL(itemSelectionChanged()), this, SLOT(templateSelectionChanged()));
}

AirframeInitialTuningPage::~AirframeInitialTuningPage()
{
    ui->templateList->clear();
    foreach(QJsonObject * templ, m_templates.values()) {
        delete templ;
    }
    m_templates.clear();

    delete ui;
}

void AirframeInitialTuningPage::initializePage()
{
    switch (getWizard()->getVehicleType()) {
    case VehicleConfigurationSource::VEHICLE_FIXEDWING:
        m_dir = VehicleTemplateExportDialog::EXPORT_FIXEDWING_NAME;
        break;
    case VehicleConfigurationSource::VEHICLE_MULTI:
        m_dir = VehicleTemplateExportDialog::EXPORT_MULTI_NAME;
        break;
    case VehicleConfigurationSource::VEHICLE_HELI:
        m_dir = VehicleTemplateExportDialog::EXPORT_HELI_NAME;
        break;
    case VehicleConfigurationSource::VEHICLE_SURFACE:
        m_dir = VehicleTemplateExportDialog::EXPORT_SURFACE_NAME;
        break;
    default:
        m_dir = NULL;
        break;
    }
    loadValidFiles();
    setupTemplateList();
}

bool AirframeInitialTuningPage::validatePage()
{
    QJsonObject *templ = NULL;

    if (ui->templateList->currentRow() >= 0) {
        templ = ui->templateList->item(ui->templateList->currentRow())->data(Qt::UserRole + 1).value<QJsonObject *>();
    }
    getWizard()->setVehicleTemplate(new QJsonObject(*templ));
    return true;
}

bool AirframeInitialTuningPage::isComplete() const
{
    return true;
}

void AirframeInitialTuningPage::updatePhoto(QJsonObject *templ)
{
    QPixmap photo;

    if (m_photoItem != NULL) {
        ui->templateImage->scene()->removeItem(m_photoItem);
    }
    if (templ != NULL) {
        QByteArray imageData = QByteArray::fromBase64(templ->value("photo").toString().toLatin1());
        photo.loadFromData(imageData, "PNG");
    } else {
        photo.load(":/core/images/opie_90x120.gif");
    }
    m_photoItem = ui->templateImage->scene()->addPixmap(photo);
    ui->templateImage->setSceneRect(ui->templateImage->scene()->itemsBoundingRect());
    ui->templateImage->fitInView(ui->templateImage->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void AirframeInitialTuningPage::updateDescription(QJsonObject *templ)
{
    if (templ != NULL) {
        QString description;
        description.append("<b>").append(tr("Name of Vehicle: ")).append("</b>").append(templ->value("name").toString()).append("<br>");
        description.append("<b>").append(tr("Name of Owner: ")).append("</b>").append(templ->value("owner").toString())
        .append(" (").append(templ->value("nick").toString()).append(")").append("<br>");
        description.append("<b>").append(tr("Size: ")).append("</b>").append(templ->value("size").toString()).append("<br>");
        description.append("<b>").append(tr("Weight: ")).append("</b>").append(templ->value("weight").toString()).append("<br>");
        description.append("<b>").append(tr("Motor(s): ")).append("</b>").append(templ->value("motor").toString()).append("<br>");
        description.append("<b>").append(tr("ESC(s): ")).append("</b>").append(templ->value("esc").toString()).append("<br>");
        description.append("<b>").append(tr("Servo(s): ")).append("</b>").append(templ->value("servo").toString()).append("<br>");
        description.append("<b>").append(tr("Battery: ")).append("</b>").append(templ->value("battery").toString()).append("<br>");
        description.append("<b>").append(tr("Propellers(s): ")).append("</b>").append(templ->value("propeller").toString()).append("<br>");
        description.append("<b>").append(tr("Controller: ")).append("</b>").append(templ->value("controller").toString()).append("<br>");
        description.append("<b>").append(tr("Comments: ")).append("</b>").append(templ->value("comment").toString());
        ui->templateDescription->setText(description);
    } else {
        ui->templateDescription->setText(tr("No vehicle selected!"));
    }
}

void AirframeInitialTuningPage::templateSelectionChanged()
{
    if (ui->templateList->currentRow() >= 0) {
        QJsonObject *templ = ui->templateList->item(ui->templateList->currentRow())->data(Qt::UserRole + 1).value<QJsonObject *>();
        updatePhoto(templ);
        updateDescription(templ);
    }
}

void AirframeInitialTuningPage::loadValidFiles()
{
    ui->templateList->clear();
    foreach(QJsonObject * templ, m_templates.values()) {
        delete templ;
    }
    m_templates.clear();

    QDir templateDir(QString("%1/%2/").arg(VehicleTemplateExportDialog::EXPORT_BASE_NAME).arg(m_dir));
    QStringList names;
    names << "*.optmpl";
    templateDir.setNameFilters(names);
    templateDir.setSorting(QDir::Name);
    QStringList files = templateDir.entryList();
    foreach(QString fileName, files) {
        QFile file(QString("%1/%2").arg(templateDir.absolutePath()).arg(fileName));

        if (file.open(QFile::ReadOnly)) {
            QByteArray jsonData = file.readAll();
            QJsonDocument templateDoc = QJsonDocument::fromJson(jsonData);
            QJsonObject json    = templateDoc.object();
            if (json["type"].toInt() == getWizard()->getVehicleType() &&
                json["subtype"].toInt() == getWizard()->getVehicleSubType()) {
                QString nameKey = getTemplateKey(&json);
                int index = 0;
                while (true) {
                    if (!m_templates.contains(nameKey)) {
                        m_templates[nameKey] = new QJsonObject(json);
                        break;
                    } else {
                        nameKey = QString("%1 - %2").arg(nameKey).arg(++index);
                    }
                }
            }
            file.close();
        }
    }
}

void AirframeInitialTuningPage::setupTemplateList()
{
    QListWidgetItem *item = new QListWidgetItem(tr("None"), ui->templateList);

    item->setData(Qt::UserRole + 1, QVariant::fromValue((QJsonObject *)NULL));
    foreach(QString templ, m_templates.keys()) {
        item = new QListWidgetItem(templ, ui->templateList);
        item->setData(Qt::UserRole + 1, QVariant::fromValue(m_templates[templ]));
    }
    ui->templateList->setCurrentRow(0);
}

QString AirframeInitialTuningPage::getTemplateKey(QJsonObject *templ)
{
    return QString("%1 - %2").arg(templ->value("nick").toString()).arg(templ->value("name").toString());
}

void AirframeInitialTuningPage::resizeEvent(QResizeEvent *)
{
    ui->templateImage->setSceneRect(ui->templateImage->scene()->itemsBoundingRect());
    ui->templateImage->fitInView(ui->templateImage->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void AirframeInitialTuningPage::showEvent(QShowEvent *)
{
    ui->templateImage->setSceneRect(ui->templateImage->scene()->itemsBoundingRect());
    ui->templateImage->fitInView(ui->templateImage->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}
