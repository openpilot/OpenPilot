#include "importexportgadgetwidget.h"
#include "ui_importexportgadgetwidget.h"
#include "coreplugin/uavgadgetinstancemanager.h"
#include "coreplugin/icore.h"
#include <QtDebug>
#include <QSettings>
#include <QMessageBox>

ImportExportGadgetWidget::ImportExportGadgetWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImportExportGadgetWidget)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    ui->setupUi(this);
}

ImportExportGadgetWidget::~ImportExportGadgetWidget()
{
    delete ui;
}

void ImportExportGadgetWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ImportExportGadgetWidget::setDialFile(const QString& filename)
{
    ui->configFile->setText(filename);
}

void ImportExportGadgetWidget::on_exportButton_clicked()
{
    QString file = ui->configFile->text();
    qDebug() << "Export pressed! Write to file " << file;
    exportConfiguration(file);

    QMessageBox msgBox;
    msgBox.setText(tr("The settings have been exported to ") + file);
    msgBox.exec();

}


void ImportExportGadgetWidget::exportConfiguration(const QString& fileName)
{
    QSettings qs(fileName, QSettings::defaultFormat());

    Core::UAVGadgetInstanceManager *im;
    im = Core::ICore::instance()->uavGadgetInstanceManager();
    im->writeConfigurations(&qs);

    qDebug() << "Export ended";
}


void ImportExportGadgetWidget::writeError(const QString& msg) const
{
    qWarning() << "ERROR: " << msg;
}

void ImportExportGadgetWidget::on_importButton_clicked()
{
    QString file = ui->configFile->text();
    qDebug() << "Import pressed! Read from file " << file;
    importConfiguration(file);

    // The new configs are added to the old configs. Things are messy now.
    QMessageBox msgBox;
    msgBox.setText(tr("The settings have been imported. Restart the application."));
    msgBox.exec();
}

void ImportExportGadgetWidget::importConfiguration(const QString& fileName)
{
    QSettings qs(fileName, QSettings::defaultFormat());

    Core::UAVGadgetInstanceManager *im;
    im = Core::ICore::instance()->uavGadgetInstanceManager();
    im->readConfigurations(&qs);

    qDebug() << "Import ended";
}
