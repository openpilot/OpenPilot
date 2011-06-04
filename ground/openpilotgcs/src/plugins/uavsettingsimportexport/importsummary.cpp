/**
 ******************************************************************************
 *
 * @file       importsummary.cpp
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
#include "importsummary.h"

ImportSummaryDialog::ImportSummaryDialog( QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportSummaryDialog)
{
   ui->setupUi(this);
   setWindowTitle(tr("Import Summary"));

   ui->importSummaryList->setColumnCount(3);
   ui->importSummaryList->setRowCount(0);
   QStringList header;
   header.append("Save");
   header.append("Name");
   header.append("Status");
   ui->importSummaryList->setHorizontalHeaderLabels(header);
   ui->progressBar->setValue(0);

   connect( ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
   connect(ui->saveToFlash, SIGNAL(clicked()), this, SLOT(doTheSaving()));
}

ImportSummaryDialog::~ImportSummaryDialog()
{
    delete ui;
}

/*
  Adds a new line about a UAVObject along with its status
  (whether it got saved OK or not)
  */
void ImportSummaryDialog::addLine(QString uavObjectName, bool status)
{
    ui->importSummaryList->setRowCount(ui->importSummaryList->rowCount()+1);
    int row = ui->importSummaryList->rowCount()-1;
    ui->progressBar->setMaximum(row);
    ui->importSummaryList->setCellWidget(row,0,new QCheckBox(ui->importSummaryList));
    QTableWidgetItem *objName = new QTableWidgetItem(uavObjectName);
    ui->importSummaryList->setItem(row, 1, objName);
    QCheckBox *box = dynamic_cast<QCheckBox*>(ui->importSummaryList->cellWidget(row,0));
    if (status) {
        ui->importSummaryList->setItem(row,2,new QTableWidgetItem("OK"));
        box->setChecked(true);
    } else {
        ui->importSummaryList->setItem(row,2,new QTableWidgetItem("Mismatch"));
        box->setChecked(false);
        box->setEnabled(false);
    }
   this->repaint();
   this->showEvent(NULL);
}

/*
  Saves every checked UAVObjet in the list to Flash
  */
void ImportSummaryDialog::doTheSaving()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVObjectUtilManager *utilManager = pm->getObject<UAVObjectUtilManager>();
    connect(utilManager, SIGNAL(saveCompleted(int,bool)), this, SLOT(updateSaveCompletion()));

    for(int i=0; i < ui->importSummaryList->rowCount(); i++) {
        QString uavObjectName = ui->importSummaryList->item(i,1)->text();
        QCheckBox *box = dynamic_cast<QCheckBox*>(ui->importSummaryList->cellWidget(i,0));
        if (box->isChecked()) {
            UAVObject* obj = objManager->getObject(uavObjectName);
            utilManager->saveObjectToSD(obj);
            this->repaint();
        }
    }
}


void ImportSummaryDialog::updateSaveCompletion()
{
    ui->progressBar->setValue(ui->progressBar->value()+1);
}

void ImportSummaryDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ImportSummaryDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    ui->importSummaryList->resizeColumnsToContents();
    int width = ui->importSummaryList->width()-(ui->importSummaryList->columnWidth(0)+
                                                ui->importSummaryList->columnWidth(2));
    ui->importSummaryList->setColumnWidth(1,width-15);
}

