/**
 ******************************************************************************
 *
 * @file       vehicletemplateselectorwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup [Group]
 * @{
 * @addtogroup VehicleTemplateSelectorWidget
 * @{
 * @brief [Brief]
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

#include "vehicletemplateselectorwidget.h"
#include "ui_vehicletemplateselectorwidget.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QDebug>
#include "vehicletemplateexportdialog.h"
#include "utils/pathutils.h"

VehicleTemplateSelectorWidget::VehicleTemplateSelectorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VehicleTemplateSelectorWidget), m_photoItem(NULL)
{
    ui->setupUi(this);
    ui->templateImage->setScene(new QGraphicsScene());
    connect(ui->templateList, SIGNAL(itemSelectionChanged()), this, SLOT(templateSelectionChanged()));
}

VehicleTemplateSelectorWidget::~VehicleTemplateSelectorWidget()
{
    ui->templateList->clear();
    foreach(QJsonObject * templ, m_templates.values()) {
        delete templ;
    }
    m_templates.clear();

    delete ui;
}

void VehicleTemplateSelectorWidget::setTemplateInfo(QString path, int vehicleType, int vehicleSubType) {
    m_templateFolder = path;
    m_vehicleType = vehicleType;
    m_vehicleSubType = vehicleSubType;
    updateTemplates();
}

QJsonObject *VehicleTemplateSelectorWidget::selectedTemplate() const
{
    if (ui->templateList->currentRow() >= 0) {
        return ui->templateList->item(ui->templateList->currentRow())->data(Qt::UserRole + 1).value<QJsonObject *>();
    }
    return NULL;
}

void VehicleTemplateSelectorWidget::updateTemplates()
{
    loadValidFiles();
    setupTemplateList();
}

void VehicleTemplateSelectorWidget::updatePhoto(QJsonObject *templ)
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

void VehicleTemplateSelectorWidget::updateDescription(QJsonObject *templ)
{
    if (templ != NULL) {
        QString description;
        description.append("<b>").append(tr("Name of Vehicle: ")).append("</b>").append(templ->value("name").toString()).append("<br>");
        description.append("<b>").append(tr("Name of Owner: ")).append("</b>").append(templ->value("owner").toString());
        if (templ->value("nick") != QStringLiteral("")) {
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

void VehicleTemplateSelectorWidget::templateSelectionChanged()
{
    if (ui->templateList->currentRow() >= 0) {
        QJsonObject *templ = selectedTemplate();
        updatePhoto(templ);
        updateDescription(templ);
    }
}

bool VehicleTemplateSelectorWidget::airframeIsCompatible(int vehicleType, int vehicleSubType)
{
    if (vehicleType != m_vehicleType) {
        return false;
    }

    return vehicleSubType == m_vehicleSubType;
}

void VehicleTemplateSelectorWidget::loadFilesInDir(QString templateBasePath)
{
    QDir templateDir(templateBasePath);

    qDebug() << "Loading templates from base path:" << templateBasePath;
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

void VehicleTemplateSelectorWidget::loadValidFiles()
{
    ui->templateList->clear();
    foreach(QJsonObject * templ, m_templates.values()) {
        delete templ;
    }
    m_templates.clear();

    loadFilesInDir(QString("%1/%2/").arg(Utils::PathUtils().InsertDataPath("%%DATAPATH%%cloudconfig")).arg(m_templateFolder));
    loadFilesInDir(QString("%1/%2/").arg(Utils::PathUtils().InsertStoragePath("%%STOREPATH%%cloudconfig")).arg(m_templateFolder));
}

void VehicleTemplateSelectorWidget::setupTemplateList()
{
    QListWidgetItem *item;

    foreach(QString templ, m_templates.keys()) {
        QJsonObject *json = m_templates[templ];

        item = new QListWidgetItem(json->value("name").toString(), ui->templateList);
        item->setData(Qt::UserRole + 1, QVariant::fromValue(json));
    }
    ui->templateList->sortItems(Qt::AscendingOrder);

    item = new QListWidgetItem(tr("Current Tuning"));
    item->setData(Qt::UserRole + 1, QVariant::fromValue((QJsonObject *)NULL));
    ui->templateList->insertItem(0, item);
    ui->templateList->setCurrentRow(0);
    // TODO Add generics to top under item Current tuning
}

QString VehicleTemplateSelectorWidget::getTemplateKey(QJsonObject *templ)
{
    return QString(templ->value("name").toString());
}

void VehicleTemplateSelectorWidget::resizeEvent(QResizeEvent *)
{
    ui->templateImage->setSceneRect(ui->templateImage->scene()->itemsBoundingRect());
    ui->templateImage->fitInView(ui->templateImage->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void VehicleTemplateSelectorWidget::showEvent(QShowEvent *)
{
    ui->templateImage->setSceneRect(ui->templateImage->scene()->itemsBoundingRect());
    ui->templateImage->fitInView(ui->templateImage->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}
