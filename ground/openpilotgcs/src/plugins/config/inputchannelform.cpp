#include "inputchannelform.h"
#include "ui_inputchannelform.h"

#include "manualcontrolsettings.h"
#include "gcsreceiver.h"

InputChannelForm::InputChannelForm(const int index, QWidget *parent) :
    ChannelForm(index, parent), ui(new Ui::InputChannelForm)
{
    ui->setupUi(this);

    connect(ui->channelMin, SIGNAL(valueChanged(int)), this, SLOT(minMaxUpdated()));
    connect(ui->channelMax, SIGNAL(valueChanged(int)), this, SLOT(minMaxUpdated()));
    connect(ui->neutralValue, SIGNAL(valueChanged(int)), this, SLOT(neutralUpdated()));
    connect(ui->channelGroup, SIGNAL(currentIndexChanged(int)), this, SLOT(groupUpdated()));
    connect(ui->channelRev, SIGNAL(toggled(bool)), this, SLOT(reversedUpdated()));

    disableMouseWheelEvents();
}

InputChannelForm::~InputChannelForm()
{
    delete ui;
}

QString InputChannelForm::name()
{
    return ui->channelName->text();
}

/**
 * Set the channel assignment label.
 */
void InputChannelForm::setName(const QString &name)
{
    ui->channelName->setText(name);
}

/**
 * Update the direction of the slider and boundaries
 */
void InputChannelForm::minMaxUpdated()
{
    bool reverse = ui->channelMin->value() > ui->channelMax->value();

    if (reverse) {
        ui->channelNeutral->setMinimum(ui->channelMax->value());
        ui->channelNeutral->setMaximum(ui->channelMin->value());
    } else {
        ui->channelNeutral->setMinimum(ui->channelMin->value());
        ui->channelNeutral->setMaximum(ui->channelMax->value());
    }
    ui->channelRev->setChecked(reverse);
    ui->channelNeutral->setInvertedAppearance(reverse);
    ui->channelNeutral->setInvertedControls(reverse);
}

void InputChannelForm::neutralUpdated()
{
    int neutralValue = ui->neutralValue->value();

    if (ui->channelRev->isChecked()) {
        if (neutralValue > ui->channelMin->value()) {
            ui->channelMin->setValue(neutralValue);
        } else if (neutralValue < ui->channelMax->value()) {
            ui->channelMax->setValue(neutralValue);
        }
    } else {
        if (neutralValue < ui->channelMin->value()) {
            ui->channelMin->setValue(neutralValue);
        } else if (neutralValue > ui->channelMax->value()) {
            ui->channelMax->setValue(neutralValue);
        }
    }
}

void InputChannelForm::reversedUpdated()
{
    int value = ui->channelNeutral->value();
    int min   = ui->channelMin->value();
    int max   = ui->channelMax->value();

    if (ui->channelRev->isChecked()) {
        if (min < max) {
            ui->channelMax->setValue(min);
            ui->channelMin->setValue(max);
            ui->channelNeutral->setValue(value);
        }
    } else {
        if (min > max) {
            ui->channelMax->setValue(min);
            ui->channelMin->setValue(max);
            ui->channelNeutral->setValue(value);
        }
    }
}

/**
 * Update the channel options based on the selected receiver type
 *
 * I fully admit this is terrible practice to embed data within UI
 * like this.  Open to suggestions. JC 2011-09-07
 */
void InputChannelForm::groupUpdated()
{
    ui->channelNumber->clear();
    ui->channelNumber->addItem("Disabled");

    quint8 count = 0;

    switch (ui->channelGroup->currentIndex()) {
    case -1: // Nothing selected
        count = 0;
        break;
    case ManualControlSettings::CHANNELGROUPS_PWM:
        count = 8; // Need to make this 6 for CC
        break;
    case ManualControlSettings::CHANNELGROUPS_PPM:
    case ManualControlSettings::CHANNELGROUPS_OPLINK:
    case ManualControlSettings::CHANNELGROUPS_DSMMAINPORT:
    case ManualControlSettings::CHANNELGROUPS_DSMFLEXIPORT:
        count = 12;
        break;
    case ManualControlSettings::CHANNELGROUPS_SBUS:
        count = 18;
        break;
    case ManualControlSettings::CHANNELGROUPS_GCS:
        count = GCSReceiver::CHANNEL_NUMELEM;
        break;
    case ManualControlSettings::CHANNELGROUPS_NONE:
        count = 0;
        break;
    default:
        Q_ASSERT(0);
    }

    for (int i = 0; i < count; i++) {
        ui->channelNumber->addItem(QString(tr("Chan %1").arg(i + 1)));
    }
}
