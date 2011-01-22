#include "importexportdialog.h"
#include "ui_importexportdialog.h"

ImportExportDialog::ImportExportDialog( ImportExportGadgetConfiguration *config, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportExportDialog)
{
    ui->setupUi(this);
    ui->widget->loadConfiguration(config);
    setWindowTitle(tr("Import Export Settings"));

    connect( ui->widget, SIGNAL(done()), this, SLOT(close()));
}

ImportExportDialog::~ImportExportDialog()
{
    delete ui;
}

void ImportExportDialog::changeEvent(QEvent *e)
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
