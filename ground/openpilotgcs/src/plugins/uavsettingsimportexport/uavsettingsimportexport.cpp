/**
 ******************************************************************************
 *
 * @file       uavsettingsimportexport.cpp
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

/*
 * TODO:
 *  - write import functions
 *  - split formats into different files/classes
 *  - better error handling (not a lot of QMessageBoxes)
 */

#include "uavsettingsimportexport.h"

#include <QtPlugin> 
#include <QStringList> 
#include <QDebug>
#include <QCheckBox>

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

UAVSettingsImportExportPlugin::UAVSettingsImportExportPlugin() 
{ 
   // Do nothing 
} 

UAVSettingsImportExportPlugin::~UAVSettingsImportExportPlugin() 
{ 
   // Do nothing 
} 

bool UAVSettingsImportExportPlugin::initialize(const QStringList& args, QString *errMsg) 
{ 
   Q_UNUSED(args); 
   Q_UNUSED(errMsg); 

   // Add Menu entry
   Core::ActionManager* am = Core::ICore::instance()->actionManager();
   Core::ActionContainer* ac = am->actionContainer(Core::Constants::M_FILE);
   Core::Command* cmd = am->registerAction(new QAction(this),
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
    cmd->action()->setText(tr("Import UAV Settings..."));
    ac->addAction(cmd, Core::Constants::G_FILE_SAVE);
    connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(importUAVSettings()));

   return true; 
} 

void UAVSettingsImportExportPlugin::extensionsInitialized() 
{ 
   // Do nothing 
}


// Slot called by the menu manager on user action
void UAVSettingsImportExportPlugin::importUAVSettings()
{
    // ask for file name
    QString fileName;
    QString filters = tr("UAVSettings XML files (*.xml)");
    fileName = QFileDialog::getOpenFileName(0, tr("Import UAV Settings"), "", filters);
    if (fileName.isEmpty()) {
        return;
    }

    // Now open the file
    QFile file(fileName);
    QDomDocument doc("UAVSettings");
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

    QDomElement root = doc.documentElement();
    if (root.tagName() != "settings") {
        QMessageBox msgBox;
        msgBox.setText(tr("Wrong file contents."));
        msgBox.setInformativeText(tr("This file is not a correct UAVSettings file"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    // We are now ok: setup the import summary dialog & update it as we
    // go along.
    ImportSummaryDialog swui;

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
         UAVObject* obj = objManager->getObject(uavObjectName);
         if (obj == NULL) {
             // This object is unknown!
             qDebug() << "Object Unknown:" << uavObjectName << uavObjectID;
             swui.addLine(uavObjectName, false);

         } else if(uavObjectID != obj->getObjID()) {
             qDebug() << "Mismatch for Object " << uavObjectName << uavObjectID << " - " << obj->getObjID();
            swui.addLine(uavObjectName, false);
         } else {
             //  - Update each field
             //  - Issue and "updated" command
             QDomNode field = node.firstChild();
             while(!field.isNull()) {
                 QDomElement f = field.toElement();
                 if (f.tagName() == "field") {
                     UAVObjectField *uavfield = obj->getField(f.attribute("name"));
                     if (f.attribute("elements").toInt() == 1) {
                         uavfield->setValue(f.attribute("values"));
                     } else {
                     // This is an enum:
                         int i=0;
                         QStringList list = f.attribute("values").split(",");
                         foreach (QString element, list) {
                             uavfield->setValue(element,i++);
                         }
                     }
                 }
                 field = field.nextSibling();
             }
             obj->updated();
             swui.addLine(uavObjectName, true);
         }
        }
        node = node.nextSibling();
    }
    swui.exec();



}

// Slot called by the menu manager on user action
void UAVSettingsImportExportPlugin::exportUAVSettings()
{
    // ask for file name
    QString fileName;
    QString filters = tr("UAVSettings XML files (*.xml)");

    fileName = QFileDialog::getSaveFileName(0, tr("Save UAV Settings File As"), "", filters);
    if (fileName.isEmpty()) {
        return;
    }

    // generate an XML first (used for all export formats as a formatted data source)
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    QDomDocument doc("UAVSettings");
    QDomElement root = doc.createElement("settings");
    doc.appendChild(root);

    // iterate over settings objects
    QList< QList<UAVDataObject*> > objList = objManager->getDataObjects();

    foreach (QList<UAVDataObject*> list, objList) {
        foreach (UAVDataObject* obj, list) {
            if (obj->isSettings()) {

                // add each object to the XML
                QDomElement o = doc.createElement("object");
                o.setAttribute("name", obj->getName());
                o.setAttribute("id", QString("0x")+ QString().setNum(obj->getObjID(),16).toUpper());
                QDomElement d = doc.createElement("description");
                QDomText t = doc.createTextNode(obj->getDescription().remove("@Ref ", Qt::CaseInsensitive));
                d.appendChild(t);
                o.appendChild(d);

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
                    f.setAttribute("type", field->getTypeAsString());
                    f.setAttribute("units", field->getUnits());
                    f.setAttribute("elements", nelem);
                    if (field->getType() == UAVObjectField::ENUM) {
                        f.setAttribute("options", field->getOptions().join(","));
                    }
                    o.appendChild(f);
                }
                root.appendChild(o);
            }
        }
    }
    // save file
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly) &&
            (file.write(doc.toString(4).toAscii()) != -1)) {
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

void UAVSettingsImportExportPlugin::shutdown() 
{ 
   // Do nothing 
}
Q_EXPORT_PLUGIN(UAVSettingsImportExportPlugin)
