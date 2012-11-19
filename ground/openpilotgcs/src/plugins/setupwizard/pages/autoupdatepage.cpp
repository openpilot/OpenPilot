#include "autoupdatepage.h"
#include "ui_autoupdatepage.h"
#include "setupwizard.h"
#include <extensionsystem/pluginmanager.h>
#include <uavobjectutil/uavobjectutilmanager.h>
#include <extensionsystem/pluginmanager.h>
#include "uploader/uploadergadgetfactory.h"

AutoUpdatePage::AutoUpdatePage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent),
    ui(new Ui::AutoUpdatePage)
{
    ui->setupUi(this);
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm);
    UploaderGadgetFactory *uploader = pm->getObject<UploaderGadgetFactory>();
    Q_ASSERT(uploader);
    connect(ui->startUpdate,SIGNAL(clicked()), this, SLOT(disableButtons()));
    connect(ui->startUpdate,SIGNAL(clicked()),uploader,SIGNAL(autoUpdate()));
    connect(uploader,SIGNAL(autoUpdateSignal(uploader::AutoUpdateStep,QVariant)),this,SLOT(updateStatus(uploader::AutoUpdateStep,QVariant)));
}

AutoUpdatePage::~AutoUpdatePage()
{
    delete ui;
}

void AutoUpdatePage::enableButtons(bool enable = false)
{
    ui->startUpdate->setEnabled(enable);
    getWizard()->button(QWizard::NextButton)->setEnabled(enable);
    getWizard()->button(QWizard::CancelButton)->setEnabled(enable);
    getWizard()->button(QWizard::BackButton)->setEnabled(enable);
    getWizard()->button(QWizard::CustomButton1)->setEnabled(enable);
    QApplication::processEvents();
}

void AutoUpdatePage::updateStatus(uploader::AutoUpdateStep status, QVariant value)
{
    switch(status)
    {
    case uploader::WAITING_DISCONNECT:
        getWizard()->setWindowFlags(getWizard()->windowFlags() & ~Qt::WindowStaysOnTopHint);
        disableButtons();
        ui->statusLabel->setText("Waiting for all OP boards to be disconnected");
        break;
    case uploader::WAITING_CONNECT:
        getWizard()->setWindowFlags(getWizard()->windowFlags() | Qt::WindowStaysOnTopHint);
        getWizard()->setWindowIcon(qApp->windowIcon());
        disableButtons();
        getWizard()->show();
        ui->statusLabel->setText("Please connect the board to the USB port (don't use external supply)");
        ui->levellinProgressBar->setValue(value.toInt());
        break;
    case uploader::JUMP_TO_BL:
        ui->levellinProgressBar->setValue(0);
        ui->statusLabel->setText("Board going into bootloader mode");
        break;
    case uploader::LOADING_FW:
        ui->statusLabel->setText("Loading firmware");
        break;
    case uploader::UPLOADING_FW:
        ui->statusLabel->setText("Uploading firmware");
        ui->levellinProgressBar->setValue(value.toInt());
        break;
    case uploader::UPLOADING_DESC:
        ui->statusLabel->setText("Uploading description");
        break;
    case uploader::BOOTING:
        ui->statusLabel->setText("Booting the board");
        break;
    case uploader::SUCCESS:
        enableButtons(true);
        ui->statusLabel->setText("Board Updated, please press the 'next' button below");
        break;
    case uploader::FAILURE:
        getWizard()->setWindowFlags(getWizard()->windowFlags() | Qt::WindowStaysOnTopHint);
        getWizard()->setWindowIcon(qApp->windowIcon());
        enableButtons(true);
        getWizard()->show();
        ui->statusLabel->setText("Something went wrong, you will have to manualy upgrade the board using the uploader plugin");
        break;
    }
}
