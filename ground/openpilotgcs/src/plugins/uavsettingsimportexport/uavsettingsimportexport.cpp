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

// for file dialog
#include <QFileDialog>

// Define USE_QFILEDIALOG to use Qt native QFileDialog() call.
// Sometimes it leaves a GUI artifact (text box with a file name) on Windows.
// If this is not defined then getSaveFileName() will be used instead.
// It calls system native dialog function which should be free of that.
//#define USE_QFILEDIALOG

// Define this to export only values w/o extra info
//#define EXPORT_VALUES_ONLY

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

// cmd->action()->setText("UAVSettings Import/Export...");
   cmd->action()->setText("UAVSettings Export...");

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
    // generate XML to export
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
#ifndef EXPORT_VALUES_ONLY
                QDomElement d = doc.createElement("description");
                QDomText t = doc.createTextNode(obj->getDescription().remove("@Ref ", Qt::CaseInsensitive));
                d.appendChild(t);
                o.appendChild(d);
#endif
                // iterate over fields
                QList<UAVObjectField*> fieldList = obj->getFields();

                foreach (UAVObjectField* field, fieldList) {
                    QDomElement f = doc.createElement("field");

#ifndef EXPORT_VALUES_ONLY
                    QString type;
                    switch (field->getType()) {
                        // TODO: extend UAVObjectField class with getTypeString() member instead of this switch
                        case UAVObjectField::INT8:
                            type = "int8";
                            break;
                        case UAVObjectField::INT16:
                            type = "int16";
                            break;
                        case UAVObjectField::INT32:
                            type = "int32";
                            break;
                        case UAVObjectField::UINT8:
                            type = "uint8";
                            break;
                        case UAVObjectField::UINT16:
                            type = "uint16";
                            break;
                        case UAVObjectField::UINT32:
                            type = "uint32";
                            break;
                        case UAVObjectField::FLOAT32:
                            type = "float32";
                            break;
                        case UAVObjectField::ENUM:
                            type = "enum";
                            break;
                        case UAVObjectField::STRING:
                            type = "string";
                            break;
                        default:
                            type = "";
                            break;
                    }
#endif
                    // iterate over values
                    QString vals;
                    quint32 nelem = field->getNumElements();
                    for (unsigned int n = 0; n < nelem; ++n) {
                        vals.append(QString("%1,").arg(field->getValue(n).toString()));
                    }
                    vals.chop(1);

                    f.setAttribute("name", field->getName());
                    f.setAttribute("values", vals);
#ifndef EXPORT_VALUES_ONLY
                    f.setAttribute("type", type);
                    f.setAttribute("units", field->getUnits());
                    f.setAttribute("elements", nelem);
#endif
                    o.appendChild(f);
                }
                root.appendChild(o);
            }
        }
    }
    QString xml = doc.toString(4);

    // save XML to a file
    QString filters = tr("UAVSettings files (*.uav);;XML files (*.xml);;All files (*)");

#ifdef USE_QFILEDIALOG
    QFileDialog *fd = new QFileDialog();
    fd->setAcceptMode(QFileDialog::AcceptSave);
    fd->setNameFilter(filters);
    fd->setDefaultSuffix("uav");

    fd->exec();
    if ((fd->result() == QFileDialog::Accepted) && (fd->selectedFiles().size() == 1)) {
        QFile file(fd->selectedFiles().first());
        if (file.open(QIODevice::WriteOnly)) {
            file.write(xml.toAscii());
            file.close();
        }
    }
#else
    QString fn = QFileDialog::getSaveFileName(0, tr("Save UAVSettings File As"), "", filters);
    if (!fn.isEmpty()) {
        QFile file(fn);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(xml.toAscii());
            file.close();
        }
    }
#endif // USE_QFILEDIALOG

}

void UAVSettingsImportExportPlugin::shutdown() 
{ 
   // Do nothing 
}
Q_EXPORT_PLUGIN(UAVSettingsImportExportPlugin)
