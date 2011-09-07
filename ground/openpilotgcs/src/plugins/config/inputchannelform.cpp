#include "inputchannelform.h"
#include "ui_inputchannelform.h"

#include "manualcontrolsettings.h"

inputChannelForm::inputChannelForm(QWidget *parent,bool showlegend) :
    QWidget(parent),
    ui(new Ui::inputChannelForm)
{
    ui->setupUi(this);
    if(!showlegend)
    {
        layout()->removeWidget(ui->legend0);
        layout()->removeWidget(ui->legend1);
        layout()->removeWidget(ui->legend2);
        layout()->removeWidget(ui->legend3);
        layout()->removeWidget(ui->legend4);
        layout()->removeWidget(ui->legend5);
        delete ui->legend0;
        delete ui->legend1;
        delete ui->legend2;
        delete ui->legend3;
        delete ui->legend4;
        delete ui->legend5;
    }

    connect(ui->channelMin,SIGNAL(valueChanged(int)),this,SLOT(minUpdated(int)));
    connect(ui->channelMax,SIGNAL(valueChanged(int)),this,SLOT(maxUpdated(int)));
    connect(ui->channelGroup,SIGNAL(currentIndexChanged(int)),this,SLOT(groupUpdated()));

    // This is awkward but since we want the UI to be a dropdown but the field is not an enum
    // it breaks the UAUVObject widget relation of the task gadget.  Running the data through
    // a spin box fixes this
    connect(ui->channelNumberDropdown,SIGNAL(currentIndexChanged(int)),this,SLOT(channelDropdownUpdated(int)));
    connect(ui->channelNumber,SIGNAL(valueChanged(int)),this,SLOT(channelNumberUpdated(int)));
}

inputChannelForm::~inputChannelForm()
{
    delete ui;
}

void inputChannelForm::minUpdated(int newval)
{
    ui->channelNeutral->setMinimum(newval);
}

void inputChannelForm::maxUpdated(int newval)
{
    ui->channelNeutral->setMaximum(newval);
}

/**
  * Update the channel options based on the selected receiver type
  *
  * I fully admit this is terrible practice to embed data within UI
  * like this.  Open to suggestions. JC 2011-09-07
  */
void inputChannelForm::groupUpdated()
{
    ui->channelNumberDropdown->clear();
    ui->channelNumberDropdown->addItem("Disabled");

    quint8 count = 0;

    switch(ui->channelGroup->currentIndex()) {
    case -1: // Nothing selected
        count = 0;
        break;
    case ManualControlSettings::CHANNELGROUPS_PWM:
        count = 8; // Need to make this 6 for CC
        break;
    case ManualControlSettings::CHANNELGROUPS_PPM:
    case ManualControlSettings::CHANNELGROUPS_SBUS:
    case ManualControlSettings::CHANNELGROUPS_SPEKTRUM1:
    case ManualControlSettings::CHANNELGROUPS_SPEKTRUM2:
        count = 12;
        break;
    case ManualControlSettings::CHANNELGROUPS_GCS:
        count = 5;
    case ManualControlSettings::CHANNELGROUPS_NONE:
        count = 0;
        break;
    default:
        Q_ASSERT(0);
    }

    for (int i = 0; i < count; i++)
        ui->channelNumberDropdown->addItem(QString(tr("Chan %1").arg(i+1)));

    ui->channelNumber->setMaximum(count);
    ui->channelNumber->setMinimum(0);
}

/**
  * Update the dropdown from the hidden control
  */
void inputChannelForm::channelDropdownUpdated(int newval)
{
    ui->channelNumber->setValue(newval);
}

/**
  * Update the hidden control from the dropdown
  */
void inputChannelForm::channelNumberUpdated(int newval)
{
    ui->channelNumberDropdown->setCurrentIndex(newval);
}
