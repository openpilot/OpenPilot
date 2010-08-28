#ifndef IMPORTEXPORTDIALOG_H
#define IMPORTEXPORTDIALOG_H

#include <QDialog>
#include "importexportgadgetconfiguration.h"

namespace Ui {
    class ImportExportDialog;
}

class ImportExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportExportDialog( ImportExportGadgetConfiguration *config, QWidget *parent = 0);
    ~ImportExportDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ImportExportDialog *ui;
};

#endif // IMPORTEXPORTDIALOG_H
