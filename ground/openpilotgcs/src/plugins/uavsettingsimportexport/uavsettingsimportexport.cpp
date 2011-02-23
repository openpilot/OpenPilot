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
                                           "UAVSettingsImportExportPlugin.UAVSettingsImportExport",
                                           QList<int>() <<
                                           Core::Constants::C_GLOBAL_ID);
   cmd->setDefaultKeySequence(QKeySequence("Ctrl+E"));

// cmd->action()->setText(tr("UAV Settings Import/Export..."));
   cmd->action()->setText(tr("UAV Settings Export..."));

// ac->menu()->addSeparator();
// ac->appendGroup("ImportExport");
// ac->addAction(cmd, "ImportExport");
   ac->addAction(cmd, Core::Constants::G_FILE_SAVE);

   connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(importExport()));

   return true; 
} 

void UAVSettingsImportExportPlugin::extensionsInitialized() 
{ 
   // Do nothing 
}

// Slot called by the menu manager on user action
// TODO: import function is not implemented yet
void UAVSettingsImportExportPlugin::importExport()
{
    // available formats
    enum { UNDEF, UAV, XML, INI } fileFormat = UNDEF;

    // ask for file name and export format
    QString fileName;
    QString filters = tr("UAV Settings files (*.uav)")
             + ";;" + tr("Simple XML files (*.xml)")
             + ";;" + tr("INI files (*.ini)");

    fileName = QFileDialog::getSaveFileName(0, tr("Save UAV Settings File As"), "", filters);
    if (fileName.isEmpty()) {
        return;
    }

    // check export file format
    QFileInfo fileInfo(fileName);
    QString fileType = fileInfo.suffix().toLower();

    if (fileType == "uav") {
        fileFormat = UAV;
    } else if (fileType == "xml") {
        fileFormat = XML;
    } else if (fileType == "ini") {
        fileFormat = INI;
    } else {
        QMessageBox::critical(0,
                              tr("UAV Settings Export"),
                              tr("Unsupported export file format: ") + fileType,
                              QMessageBox::Ok);
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
                if (fileFormat == UAV) {
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
                    if (fileFormat == UAV) {
                        f.setAttribute("type", field->getTypeAsString());
                        f.setAttribute("units", field->getUnits());
                        f.setAttribute("elements", nelem);
                        if (field->getType() == UAVObjectField::ENUM) {
                            f.setAttribute("options", field->getOptions().join(","));
                        }
                    }
                    o.appendChild(f);
                }
                root.appendChild(o);
            }
        }
    }

    // save file
    if ((fileFormat == UAV) || (fileFormat == XML)) {
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
    } else if (fileFormat == INI) {
        if (QFile::exists(fileName) && !QFile::remove(fileName)) {
            QMessageBox::critical(0,
                                  tr("UAV Settings Export"),
                                  tr("Unable to remove existing file: ") + fileName,
                                  QMessageBox::Ok);
            return;
        }

        QSettings ini(fileName, QSettings::IniFormat);
        QDomElement docElem = doc.documentElement();
        QDomNodeList nodeList = docElem.elementsByTagName("object");
        for (int i = 0; i < nodeList.count(); i++) {
            QDomElement e = nodeList.at(i).toElement();
            if (!e.isNull()) {
                ini.beginGroup(e.attribute("name", "undefined"));
                ini.setValue("id", e.attribute("id"));
                QDomNodeList n = e.elementsByTagName("field");
                for (int j = 0; j < n.count(); j++) {
                    QDomElement f = n.at(j).toElement();
                    if (!f.isNull()) {
                        ini.setValue(f.attribute("name", "unknown"), f.attribute("values"));
                    }
                }
                ini.endGroup();
            }
        }
    }
}

void UAVSettingsImportExportPlugin::shutdown() 
{ 
   // Do nothing 
}
Q_EXPORT_PLUGIN(UAVSettingsImportExportPlugin)
