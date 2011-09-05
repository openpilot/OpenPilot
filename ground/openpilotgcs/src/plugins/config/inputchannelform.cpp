#include "inputchannelform.h"
#include "ui_inputchannelform.h"

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
}

inputChannelForm::~inputChannelForm()
{
    delete ui;
}
