/**
 ******************************************************************************
 *
 * @file       uavsettingsimportexportfactory.cpp
 * @author     (C) 2011 The OpenPilot Team, http://www.openpilot.org
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVSettingsImportExport UAVSettings Import/Export Plugin
 * @{
 * @brief UAVSettings Import/Export Plugin
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

#include "uavsettingsimportexportfactory.h"
#include <QtPlugin>
#include <QStringList>
#include <QDebug>
#include <QCheckBox>
#include "importsummary.h"

// for menu item
#include <coreplugin/coreconstants.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/icore.h>
#include <QKeySequence>

// for UAVObjects
#include "uavdataobject.h"
#include "uavobjectmanager.h"
#include "extensionsystem/pluginmanager.h"

// for XML object
#include <QDomDocument>

// for file dialog and error messages
#include <QFileDialog>
#include <QMessageBox>

UAVSettingsImportExportFactory::~UAVSettingsImportExportFactory()
{
   // Do nothing
}

UAVSettingsImportExportFactory::UAVSettingsImportExportFactory(QObject *parent):QObject(parent)
{

   // Add Menu entry
   Core::ActionManager *am = Core::ICore::instance()->actionManager();
   Core::ActionContainer *ac = am->actionContainer(Core::Constants::M_FILE);
   Core::Command *cmd = am->registerAction(new QAction(this),
                                           "UAVSettingsImportExportPlugin.UAVSettingsExport",
                                           QList<int>() <<
                                           Core::Constants::C_GLOBAL_ID);
    cmd->setDefaultKeySequence(QKeySequence("Ctrl+E"));
    cmd->action()->setText(tr("Export UAV Settings..."));
    ac->addAction(cmd, Core::Constants::G_FILE_SAVE);
    connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(exportUAVSettings()));

    cmd = am->registerAction(new QAction(this),
                             "UAVSettingsImportExportPlugin.UAVSettingsImport",
                             QList<int>() <<
                             Core::Constants::C_GLOBAL_ID);
    cmd->setDefaultKeySequence(QKeySequence("Ctrl+I"));
    cmd->action()->setText(tr("Import UAV Settings..."));
    ac->addAction(cmd, Core::Constants::G_FILE_SAVE);
    connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(importUAVSettings()));

    ac = am->actionContainer(Core::Constants::M_HELP);
    cmd = am->registerAction(new QAction(this),
                             "UAVSettingsImportExportPlugin.UAVDataExport",
                             QList<int>() <<
                             Core::Constants::C_GLOBAL_ID);
    cmd->action()->setText(tr("Export UAV Data..."));
    ac->addAction(cmd, Core::Constants::G_HELP_HELP);
    connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(exportUAVData()));
}

// Slot called by the menu manager on user action
void UAVSettingsImportExportFactory::importUAVSettings()
{
    // ask for file name
    QString fileName;
    QString filters = tr("UAVObjects XML files (*.uav);; XML files (*.xml)");
    fileName = QFileDialog::getOpenFileName(0, tr("Import UAV Settings"), "", filters);
    if (fileName.isEmpty()) {
        return;
    }

    // Now open the file
    QFile file(fileName);
    QDomDocument doc("UAVObjects");
    file.open(QFile::ReadOnly|QFile::Text);
    if (!doc.setContent(file.readAll())) {
        QMessageBox msgBox;
        msgBox.setText(tr("File Parsing Failed."));
        msgBox.setInformativeText(tr("This file is not a correct XML file"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }
    file.close();

    // find the root of settings subtree
    emit importAboutToBegin();
    qDebug()<<"Import about to begin";

    QDomElement root = doc.documentElement();
    if (root.tagName() == "uavobjects") {
        root = root.firstChildElement("settings");
    }
    if (root.isNull() || (root.tagName() != "settings")) {
        QMessageBox msgBox;
        msgBox.setText(tr("Wrong file contents"));
        msgBox.setInformativeText(tr("This file does not contain correct UAVSettings"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    // We are now ok: setup the import summary dialog & update it as we
    // go along.
    ImportSummaryDialog swui((QWidget*)Core::ICore::instance()->mainWindow());

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    swui.show();

    QDomNode node = root.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (e.tagName() == "object") {

            //  - Read each object
            QString uavObjectName  = e.attribute("name");
            uint uavObjectID = e.attribute("id").toUInt(NULL,16);

            // Sanity Check:
            UAVObject *obj = objManager->getObject(uavObjectName);
            if (obj == NULL) {
                // This object is unknown!
                qDebug() << "Object unknown:" << uavObjectName << uavObjectID;
                swui.addLine(uavObjectName, "Error (Object unknown)", false);
            } else {
                //  - Update each field
                //  - Issue and "updated" command
                bool error = false;
                bool setError = false;
                QDomNode field = node.firstChild();
                while(!field.isNull()) {
                    QDomElement f = field.toElement();
                    if (f.tagName() == "field") {
                        UAVObjectField *uavfield = obj->getField(f.attribute("name"));
                        if (uavfield) {
                            QStringList list = f.attribute("values").split(",");
                            if (list.length() == 1) {
                                if (false == uavfield->checkValue(f.attribute("values"))) {
                                    qDebug() << "checkValue returned false on: " << uavObjectName << f.attribute("values");
                                    setError = true;
                                } else {
                                    uavfield->setValue(f.attribute("values"));
                                }
                            } else {
                                // This is an enum:
                                int i = 0;
                                QStringList list = f.attribute("values").split(",");
                                foreach (QString element, list) {
                                    if (false == uavfield->checkValue(element, i)) {
                                        qDebug() << "checkValue(list) returned false on: " << uavObjectName << list;
                                        setError = true;
                                    } else {
                            		 uavfield->setValue(element,i);
                                    }
                                    i++;
                                }
                            }
                        } else {
                            error = true;
                        }
                    }
                    field = field.nextSibling();
                }
                obj->updated();

                if (error) {
                    swui.addLine(uavObjectName, "Warning (Object field unknown)", true);
                } else if (uavObjectID != obj->getObjID()) {
                    qDebug() << "Mismatch for Object " << uavObjectName << uavObjectID << " - " << obj->getObjID();
                    swui.addLine(uavObjectName, "Warning (ObjectID mismatch)", true);
                } else if (setError) {
                    swui.addLine(uavObjectName, "Warning (Objects field value(s) invalid)", false);
                } else {
                    swui.addLine(uavObjectName, "OK", true);
                }
            }
        }
        node = node.nextSibling();
    }
    qDebug() << "End import";
    swui.exec();
}

// Create an XML document from UAVObject database
QString UAVSettingsImportExportFactory::createXMLDocument(const enum storedData what, const bool fullExport)
{
    // generate an XML first (used for all export formats as a formatted data source)
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    // create an XML root
    QDomDocument doc("UAVObjects");
    QDomElement root = doc.createElement("uavobjects");
    doc.appendChild(root);

    // add hardware, firmware and GCS version info
    QDomElement versionInfo = doc.createElement("version");
    root.appendChild(versionInfo);

    UAVObjectUtilManager *utilMngr = pm->getObject<UAVObjectUtilManager>();
    deviceDescriptorStruct board = utilMngr->getBoardDescriptionStruct();

    QDomElement hw = doc.createElement("hardware");
    hw.setAttribute("type", QString().setNum(board.boardType, 16));
    hw.setAttribute("revision", QString().setNum(board.boardRevision, 16));
    hw.setAttribute("serial", QString(utilMngr->getBoardCPUSerial().toHex()));
    versionInfo.appendChild(hw);

    QDomElement fw = doc.createElement("firmware");
    fw.setAttribute("date", board.gitDate);
    fw.setAttribute("hash", board.gitHash);
    fw.setAttribute("tag", board.gitTag);
    versionInfo.appendChild(fw);

    QString gcsRevision = QString::fromLatin1(Core::Constants::GCS_REVISION_STR);
    QString gcsGitDate = gcsRevision.mid(gcsRevision.indexOf(" ") + 1, 14);
    QString gcsGitHash = gcsRevision.mid(gcsRevision.indexOf(":") + 1, 8);
    QString gcsGitTag = gcsRevision.left(gcsRevision.indexOf(":"));

    QDomElement gcs = doc.createElement("gcs");
    gcs.setAttribute("date", gcsGitDate);
    gcs.setAttribute("hash", gcsGitHash);
    gcs.setAttribute("tag", gcsGitTag);
    versionInfo.appendChild(gcs);

    // create settings and/or data elements
    QDomElement settings = doc.createElement("settings");
    QDomElement data = doc.createElement("data");

    switch (what)
    {
    case Settings:
        root.appendChild(settings);
        break;
    case Data:
        root.appendChild(data);
        break;
    case Both:
        root.appendChild(data);
        root.appendChild(settings);
        break;
    }

    // iterate over settings objects
    QList< QList<UAVDataObject*> > objList = objManager->getDataObjects();
    foreach (QList<UAVDataObject*> list, objList) {
        foreach (UAVDataObject *obj, list) {
            if (((what == Settings) && obj->isSettings()) ||
                ((what == Data) && !obj->isSettings()) ||
                 (what == Both)) {

                // add each object to the XML
                QDomElement o = doc.createElement("object");
                o.setAttribute("name", obj->getName());
                o.setAttribute("id", QString("0x")+ QString().setNum(obj->getObjID(),16).toUpper());
                if (fullExport) {
                    QDomElement d = doc.createElement("description");
                    QDomText t = doc.createTextNode(obj->getDescription().remove("@Ref ", Qt::CaseInsensitive));
                    d.appendChild(t);
                    o.appendChild(d);
                }

                // iterate over fields
                QList<UAVObjectField*> fieldList = obj->getFields();

                foreach (UAVObjectField* field, fieldList) {
                    QDomElement f = doc.createElement("field");

                    // iterate over values
                    QString vals;
                    quint32 nelem = field->getNumElements();
                    for (unsigned int n = 0; n < nelem; ++n) {
                        vals.append(QString("%1,").arg(field->getValue(n).toString()));
                    }
                    vals.chop(1);

                    f.setAttribute("name", field->getName());
                    f.setAttribute("values", vals);
                    if (fullExport) {
                        f.setAttribute("type", field->getTypeAsString());
                        f.setAttribute("units", field->getUnits());
                        f.setAttribute("elements", nelem);
                        if (field->getType() == UAVObjectField::ENUM) {
                            f.setAttribute("options", field->getOptions().join(","));
                        }
                    }
                    o.appendChild(f);
                }

                // append to the settings or data element
                if (obj->isSettings())
                    settings.appendChild(o);
                else
                    data.appendChild(o);
            }
        }
    }

    return doc.toString(4);
}

// Slot called by the menu manager on user action
void UAVSettingsImportExportFactory::exportUAVSettings()
{
    // ask for file name
    QString fileName;
    QString filters = tr("UAVObjects XML files (*.uav)");

    fileName = QFileDialog::getSaveFileName(0, tr("Save UAVSettings File As"), "", filters);
    if (fileName.isEmpty()) {
        return;
    }

    // If the filename ends with .xml, we will do a full export, otherwise, a simple export
    bool fullExport = false;
    if (fileName.endsWith(".xml")) {
        fullExport = true;
    } else if (!fileName.endsWith(".uav")) {
        fileName.append(".uav");
    }

    // generate an XML first (used for all export formats as a formatted data source)
    QString xml = createXMLDocument(Settings, fullExport);

    // save file
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly) &&
            (file.write(xml.toAscii()) != -1)) {
        file.close();
    } else {
        QMessageBox::critical(0,
                              tr("UAV Settings Export"),
                              tr("Unable to save settings: ") + fileName,
                              QMessageBox::Ok);
        return;
    }

    QMessageBox msgBox;
    msgBox.setText(tr("Settings saved."));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

// Slot called by the menu manager on user action
void UAVSettingsImportExportFactory::exportUAVData()
{
    if (QMessageBox::question(0, tr("Are you sure?"),
        tr("This option is only useful for passing your current "
           "system data to the technical support staff. "
           "Do you really want to export?"),
           QMessageBox::Ok | QMessageBox::Cancel,
           QMessageBox::Ok) != QMessageBox::Ok) {
        return;
    }

    // ask for file name
    QString fileName;
    QString filters = tr("UAVObjects XML files (*.uav)");

    fileName = QFileDialog::getSaveFileName(0, tr("Save UAVData File As"), "", filters);
    if (fileName.isEmpty()) {
        return;
    }

    // If the filename ends with .xml, we will do a full export, otherwise, a simple export
    bool fullExport = false;
    if (fileName.endsWith(".xml")) {
        fullExport = true;
    } else if (!fileName.endsWith(".uav")) {
        fileName.append(".uav");
    }

    // generate an XML first (used for all export formats as a formatted data source)
    QString xml = createXMLDocument(Both, fullExport);

    // save file
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly) &&
            (file.write(xml.toAscii()) != -1)) {
        file.close();
    } else {
        QMessageBox::critical(0,
                              tr("UAV Data Export"),
                              tr("Unable to save data: ") + fileName,
                              QMessageBox::Ok);
        return;
    }

    QMessageBox msgBox;
    msgBox.setText(tr("Data saved."));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}
