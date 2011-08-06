/**
 ******************************************************************************
 * @file
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup importexportplugin
 * @{
 *****************************************************************************/
#ifndef IMPORTEXPORTGADGETWIDGET_H
#define IMPORTEXPORTGADGETWIDGET_H

#include <QWidget>
#include <QString>
#include "importexport_global.h"
#include <coreplugin/iconfigurableplugin.h>

namespace Ui
{
class ImportExportGadgetWidget;
}

class IMPORTEXPORT_EXPORT ImportExportGadgetWidget : public QWidget
{
    Q_OBJECT
public:
    ImportExportGadgetWidget(QWidget *parent = 0);
    ~ImportExportGadgetWidget();

signals:
    void done();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ImportExportGadgetWidget *ui;
    void writeError(const QString&) const;
    void exportConfiguration(const QString& fileName);
    void importConfiguration(const QString& fileName);
    QList<Core::IConfigurablePlugin*> getConfigurables();

	QString filename;

private slots:
	void on_resetButton_clicked();
    void on_helpButton_clicked();
    void on_importButton_clicked();
    void on_exportButton_clicked();
};

#endif // IMPORTEXPORTGADGETWIDGET_H
/**
 * @}
 * @}
 */
