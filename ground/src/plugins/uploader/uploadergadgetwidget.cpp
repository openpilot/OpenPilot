/**
 ******************************************************************************
 *
 * @file       UploaderGadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   Uploader
 * @{
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include "uploadergadgetwidget.h"

UploaderGadgetWidget::UploaderGadgetWidget(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    QVBoxLayout *layout = new QVBoxLayout;

    QHBoxLayout *FileLayout = new QHBoxLayout;
    QWidget *FileWidget = new QWidget;
    FileWidget->setLayout(FileLayout);
    openFileNameLE=new QLineEdit();
    QPushButton* loadfile = new QPushButton("Load File");
    loadfile->setMaximumWidth(80);
    FileLayout->addWidget(openFileNameLE);
    FileLayout->addWidget(loadfile);


    QHBoxLayout *SendLayout = new QHBoxLayout;
    QWidget *SendWidget = new QWidget;
    SendWidget->setLayout(SendLayout);
    progressBar=new QProgressBar();
    progressBar->setMaximum(100);
    QPushButton* sendBt = new QPushButton("Send");
    sendBt->setMaximumWidth(80);
    SendLayout->addWidget(progressBar);
    SendLayout->addWidget(sendBt);

    QHBoxLayout *StatusLayout = new QHBoxLayout;
    QWidget *StatusWidget = new QWidget;
    StatusWidget->setLayout(StatusLayout);
    status=new QLabel();
    StatusLayout->addWidget(status);

    layout->addWidget(FileWidget);
    layout->addWidget(SendWidget);
    layout->addWidget(StatusWidget);
    setLayout(layout);

    timer = new QTimer(this);

    connect(timer, SIGNAL(timeout()),
            this, SLOT(updatePerc()));
    connect(loadfile, SIGNAL(clicked(bool)),
            this,SLOT(setOpenFileName()));
    connect(sendBt, SIGNAL(clicked(bool)),
            this,SLOT(send()));



}


void UploaderGadgetWidget::updatePerc()
{
    if(!Ymodem->isRunning())
    {
         timer->stop();
    }
    progressBar->setValue(percent);
}

void UploaderGadgetWidget::send()
{
    Ymodem->SendFileT(openFileNameLE->text());
    timer->start(500);
}

UploaderGadgetWidget::~UploaderGadgetWidget()
{
    delete Port;
    delete Ymodem;
}

void UploaderGadgetWidget::setPort(QextSerialPort* port)
{
    Port=port;
    Ymodem=new QymodemSend(*Port);
    connect(Ymodem,SIGNAL(Error(QString,int))
            ,this,SLOT(error(QString,int)));
    connect(Ymodem,SIGNAL(Information(QString,int)),
            this,SLOT(info(QString,int)));
    connect(Ymodem,SIGNAL(Percent(int)),
            this,SLOT(updatePercSlot(int)));
}

void UploaderGadgetWidget::updatePercSlot(int i)
{
    percent=i;
}

void UploaderGadgetWidget::setOpenFileName()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("QFileDialog::getOpenFileName()"),
                                                    openFileNameLE->text(),
                                                    tr("All Files (*);;Text Files (*.bin)"),
                                                    &selectedFilter,
                                                    options);
    if (!fileName.isEmpty()) openFileNameLE->setText(fileName);

}

void UploaderGadgetWidget::error(QString errorString, int errorNumber)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(errorString);
    msgBox.exec();
    status->setText(errorString);
}
void UploaderGadgetWidget::info(QString infoString, int infoNumber)
{
    status->setText(infoString);
}
