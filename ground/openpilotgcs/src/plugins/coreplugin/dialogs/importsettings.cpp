#include "importsettings.h"
#include "ui_importsettings.h"
#include <QDir>
#include <QSettings>
#include <utils/xmlconfig.h>
#include <QTimer>

importSettings::importSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::importSettings)
{
    ui->setupUi(this);
    connect(ui->cbConfigs,SIGNAL(currentIndexChanged(int)),this,SLOT(updateDetails(int)));
    connect(ui->btnLoad,SIGNAL(clicked()),this,SLOT(accept()));
    QTimer::singleShot(500,this,SLOT(repaint()));
}
void importSettings::loadFiles(QString path)
{
    QDir myDir(path);
    QStringList filters;
    filters << "*.xml";
    QStringList list = myDir.entryList(filters,QDir::Files);
    int x=0;
    foreach(QString fileStr, list)
    {
        fileInfo * info=new fileInfo;
        QSettings settings(path+QDir::separator()+fileStr, XmlConfig::XmlSettingsFormat);
        settings.beginGroup("General");
        info->description=settings.value("Description","None").toString();
        info->details=settings.value("Details","None").toString();
        settings.endGroup();
        info->file=path+QDir::separator()+fileStr;
        configList.insert(x,info);
        ui->cbConfigs->addItem(info->description,x);
        ++x;
    }
}

void importSettings::updateDetails(int index)
{
    fileInfo * info=configList.value(ui->cbConfigs->itemData(index).toInt());
    ui->lblDetails->setText(info->details);
}
QString importSettings::choosenConfig()
{
    fileInfo * info=configList.value(ui->cbConfigs->itemData(ui->cbConfigs->currentIndex()).toInt());
    return info->file;
}

importSettings::~importSettings()
{
    foreach(fileInfo * info,configList.values())
    {
        delete info;
    }
    delete ui;
}
