/**
 ******************************************************************************
 *
 * @file       airframeinitialtuningpage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup
 * @{
 * @addtogroup AirframeInitialTuningPage
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
    if (getWizard()->getVehicleTemplate() != NULL) {
        delete getWizard()->getVehicleTemplate();
    }
    getWizard()->setVehicleTemplate(templ != NULL ? new QJsonObject(*templ) : NULL);
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
    if (templ != NULL && !templ->value("photo").isUndefined()) {
        QByteArray imageData = QByteArray::fromBase64(templ->value("photo").toString().toLatin1());
        photo.loadFromData(imageData, "PNG");
    } else {
        photo.load(":/core/images/openpilot_logo_500.png");
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
        description.append("<b>").append(tr("Name of Owner: ")).append("</b>").append(templ->value("owner").toString());
        if (templ->value("nick") != "") {
            description.append(" (").append(templ->value("nick").toString()).append(")");
        }
        description.append("<br>");
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
        ui->templateDescription->setText(tr("This option will use the current tuning settings saved on the controller, if your controller "
                                            "is currently unconfigured, then the OpenPilot firmware defaults will be used.\n\n"
                                            "It is suggested that if this is a first time configuration of your controller, rather than "
                                            "use this option, instead select a tuning set that matches your own airframe as close as "
                                            "possible from the list above or if you are not able to fine one, then select the generic item "
                                            "from the list."));
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

bool AirframeInitialTuningPage::airframeIsCompatible(int vehicleType, int vehicleSubType)
{
    if (vehicleType != getWizard()->getVehicleType()) {
        return false;
    }

    int wizSubType = getWizard()->getVehicleSubType();
    switch (vehicleType) {
        case VehicleConfigurationSource::MULTI_ROTOR_QUAD_H:
        case VehicleConfigurationSource::MULTI_ROTOR_QUAD_X:
        {
            return wizSubType == VehicleConfigurationSource::MULTI_ROTOR_QUAD_H ||
                    wizSubType == VehicleConfigurationSource::MULTI_ROTOR_QUAD_X;
        }
        default:
            return vehicleSubType == wizSubType;
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
            QJsonParseError error;
            QJsonDocument templateDoc = QJsonDocument::fromJson(jsonData, &error);
            if (error.error == QJsonParseError::NoError) {
                QJsonObject json = templateDoc.object();
                if (airframeIsCompatible(json["type"].toInt(), json["subtype"].toInt())) {
                    QString uuid = json["uuid"].toString();
                    if (!m_templates.contains(uuid)) {
                        m_templates[json["uuid"].toString()] = new QJsonObject(json);
                    }
                }
            } else {
                qDebug() << "Error parsing json file: "
                         << fileName << ". Error was:" << error.errorString();
            }
        }
        file.close();
    }
}

void AirframeInitialTuningPage::setupTemplateList()
{
    QListWidgetItem *item = new QListWidgetItem(tr("Current Tuning"), ui->templateList);

    item->setData(Qt::UserRole + 1, QVariant::fromValue((QJsonObject *)NULL));
    foreach(QString templ, m_templates.keys()) {
        QJsonObject *json = m_templates[templ];

        item = new QListWidgetItem(json->value("name").toString(), ui->templateList);
        item->setData(Qt::UserRole + 1, QVariant::fromValue(json));
    }
    ui->templateList->setCurrentRow(0);
}

QString AirframeInitialTuningPage::getTemplateKey(QJsonObject *templ)
{
    return QString(templ->value("name").toString());
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
