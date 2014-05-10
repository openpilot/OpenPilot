#include "inputchannelform.h"
#include "ui_inputchannelform.h"

#include "manualcontrolsettings.h"
#include "gcsreceiver.h"

InputChannelForm::InputChannelForm(QWidget *parent) : ConfigTaskWidget(parent), ui(new Ui::InputChannelForm)
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

void InputChannelForm::addToGrid(QGridLayout *gridLayout)
{
    // if we are the first row to be inserted the show the legend
    bool showLegend = (gridLayout->rowCount() == 1);

    // The first time through the loop, keep the legend. All other times, delete it.
    if (false && !showLegend) {
        QLayout *legendLayout = layout()->itemAt(0)->layout();
        Q_ASSERT(legendLayout);
        // remove every item
        while (legendLayout->count()) {
            QLayoutItem *item = legendLayout->takeAt(0);
            if (!item) {
                continue;
            }
            // get widget from layout item
            QWidget *widget = item->widget();
            if (widget) {
                delete widget;
                continue;
            }
        }
        // and finally remove and delete the legend layout
        layout()->removeItem(legendLayout);
        delete legendLayout;
    }

    QGridLayout *srcLayout = dynamic_cast<QGridLayout*>(layout());
    Q_ASSERT(srcLayout);

    if (showLegend) {
        Q_ASSERT(srcLayout);
        int row = gridLayout->rowCount();
        for(int col = 0; col < srcLayout->columnCount(); col++) {
            QLayoutItem *item = srcLayout->itemAtPosition(0, col);
            if (!item) {
                continue;
            }
            QWidget *widget = item->widget();
            if (widget) {
                gridLayout->addWidget(widget, row, col);
                continue;
            }
        }
    }

    int row = gridLayout->rowCount();
    for(int col = 0; col < srcLayout->columnCount(); col++) {
        QLayoutItem *item = srcLayout->itemAtPosition(1, col);
        if (!item) {
            continue;
        }
        QWidget *widget = item->widget();
        if (widget) {
            gridLayout->addWidget(widget, row, col);
            continue;
        }
    }

    //
    setVisible(false);
}

void InputChannelForm::setName(QString &name)
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
