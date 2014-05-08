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
    UploaderGadgetFactory *uploader    = pm->getObject<UploaderGadgetFactory>();
    Q_ASSERT(uploader);
    connect(ui->startUpdate, SIGNAL(clicked()), this, SLOT(disableButtons()));
    connect(ui->startUpdate, SIGNAL(clicked()), uploader, SIGNAL(autoUpdate()));
    connect(uploader, SIGNAL(autoUpdateSignal(uploader::AutoUpdateStep, QVariant)), this, SLOT(updateStatus(uploader::AutoUpdateStep, QVariant)));
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
    QString msg;

    switch (status) {
    case uploader::WAITING_DISCONNECT:
        disableButtons();
        ui->statusLabel->setText(tr("Waiting for all OP boards to be disconnected."));
        // TODO get rid of magic number 20s (should use UploaderGadgetWidget::BOARD_EVENT_TIMEOUT)
        ui->levellinProgressBar->setMaximum(20);
        ui->levellinProgressBar->setValue(value.toInt());
        break;
    case uploader::WAITING_CONNECT:
        // Note:
        // the following commented out lines were probably added to fix an issue when uploader opened a popup requesting
        // user to disconnect all boards
        // Side effect is that the wizard dialog flickers
        // the uploader was changed to avoid popups alltogether and that fix is not need anymore
        // same commented fix can be found in FAILURE case and they are kept for future ref.
        // getWizard()->setWindowFlags(getWizard()->windowFlags() | Qt::WindowStaysOnTopHint);
        // getWizard()->setWindowIcon(qApp->windowIcon());
        // getWizard()->show();
        // End of Note
        disableButtons();
        ui->statusLabel->setText(tr("Please connect the board to the USB port (don't use external supply)."));
        // TODO get rid of magic number 20s (should use UploaderGadgetWidget::BOARD_EVENT_TIMEOUT)
        ui->levellinProgressBar->setMaximum(20);
        ui->levellinProgressBar->setValue(value.toInt());
        break;
    case uploader::JUMP_TO_BL:
        ui->levellinProgressBar->setValue(0);
        ui->statusLabel->setText(tr("Board going into bootloader mode."));
        break;
    case uploader::LOADING_FW:
        ui->statusLabel->setText(tr("Loading firmware."));
        break;
    case uploader::UPLOADING_FW:
        ui->statusLabel->setText(tr("Uploading firmware."));
        ui->levellinProgressBar->setMaximum(100);
        ui->levellinProgressBar->setValue(value.toInt());
        break;
    case uploader::UPLOADING_DESC:
        ui->statusLabel->setText(tr("Uploading description."));
        break;
    case uploader::BOOTING:
        ui->statusLabel->setText(tr("Booting the board."));
        break;
    case uploader::SUCCESS:
        enableButtons(true);
        ui->statusLabel->setText(tr("Board updated, please press 'Next' to continue."));
        break;
    case uploader::FAILURE:
        // getWizard()->setWindowFlags(getWizard()->windowFlags() | Qt::WindowStaysOnTopHint);
        // getWizard()->setWindowIcon(qApp->windowIcon());
        enableButtons(true);
        QString msg = value.toString();
        if (msg.isEmpty()) {
            msg = tr("Something went wrong, you will have to manually upgrade the board using the uploader plugin.");
        }
        ui->statusLabel->setText(QString("<font color='red'>%1</font>").arg(msg));
        break;
    }
}
