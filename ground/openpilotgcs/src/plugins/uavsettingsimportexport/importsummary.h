/**
 ******************************************************************************
 *
 * @file       importsummary.h
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
#ifndef IMPORTSUMMARY_H
#define IMPORTSUMMARY_H

#include <QDialog>
#include <QCheckBox>
#include <QDesktopServices>
#include <QUrl>
#include "ui_importsummarydialog.h"
#include "uavdataobject.h"
#include "uavobjectmanager.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectutil/uavobjectutilmanager.h"



namespace Ui {
    class ImportSummaryDialog;
}

class ImportSummaryDialog : public QDialog
{
    Q_OBJECT

public:
    ImportSummaryDialog(QWidget *parent=0);
    ~ImportSummaryDialog();
    void addLine(QString objectName, QString text, bool status);

protected:
    void showEvent(QShowEvent *event);
    void changeEvent(QEvent *e);

private:
    Ui::ImportSummaryDialog *ui;

public slots:
    void updateSaveCompletion();

private slots:
    void doTheSaving();
    void openHelp();

};

#endif // IMPORTSUMMARY_H
