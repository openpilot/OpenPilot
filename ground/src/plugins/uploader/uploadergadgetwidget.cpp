/**
 ******************************************************************************
 *
 * @file       uploadergadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Uploader Plugin Gadget widget
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   uploaderplugin
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

    //main layout
    QVBoxLayout *layout = new QVBoxLayout;
    //choose file layout and widget
    QHBoxLayout *FileLayout = new QHBoxLayout;
    QWidget *FileWidget = new QWidget;
    FileWidget->setLayout(FileLayout);
    openFileNameLE=new QLineEdit();
    QPushButton* loadfile = new QPushButton("Load File");
    loadfile->setMaximumWidth(80);
    FileLayout->addWidget(openFileNameLE);
    FileLayout->addWidget(loadfile);

    //send file layout and widget
    QHBoxLayout *SendLayout = new QHBoxLayout;
    QWidget *SendWidget = new QWidget;
    SendWidget->setLayout(SendLayout);
    progressBar=new QProgressBar();
    progressBar->setMaximum(100);
    QPushButton* sendBt = new QPushButton("Send");
    sendBt->setMaximumWidth(80);
    SendLayout->addWidget(progressBar);
    SendLayout->addWidget(sendBt);

    //status layout and widget
    QHBoxLayout *StatusLayout = new QHBoxLayout;
    QWidget *StatusWidget = new QWidget;
    StatusWidget->setLayout(StatusLayout);
    status=new QLabel();
    StatusLayout->addWidget(status);

    //add partial widgets to main widget
    layout->addWidget(FileWidget);
    layout->addWidget(SendWidget);
    layout->addWidget(StatusWidget);
    setLayout(layout);

    //connect signals to slots

    //fires when the user presses file button
    connect(loadfile, SIGNAL(clicked(bool)),
            this,SLOT(setOpenFileName()));
    //fires when the user presses send button
    connect(sendBt, SIGNAL(clicked(bool)),
            this,SLOT(send()));
}
//user pressed send, send file using a new thread with qymodem library
void UploaderGadgetWidget::send()
{
    Ymodem->SendFileT(openFileNameLE->text());
}
//destructor !!?! do I need to delete something else?
UploaderGadgetWidget::~UploaderGadgetWidget()
{
    delete Port;
    delete Ymodem;
}

//from load configuration, creates a new qymodemsend class with the the port
/**
Cteates a new qymodemsend class.

@param port	The serial port to use.


*/
void UploaderGadgetWidget::setPort(QextSerialPort* port)
{

    Port=port;
    Ymodem=new QymodemSend(*Port);
    //only now can we connect this signals
    //signals errors
    connect(Ymodem,SIGNAL(Error(QString,int))
            ,this,SLOT(error(QString,int)));
    //signals new information
    connect(Ymodem,SIGNAL(Information(QString,int)),
            this,SLOT(info(QString,int)));
    //signals new percentage value
    connect(Ymodem,SIGNAL(Percent(int)),
            this,SLOT(updatePercSlot(int)));
}
/**
Updates progress bar value.

@param i	New percentage value.

*/
void UploaderGadgetWidget::updatePercSlot(int i)
{
    progressBar->setValue(i);
}
/**

Opens an open file dialog.

*/
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
/**
Shows a message box with an error string.

@param errorString	The error string to display.

@param errorNumber      Not used

*/
void UploaderGadgetWidget::error(QString errorString, int errorNumber)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(errorString);
    msgBox.exec();
    status->setText(errorString);
}
/**
Shows a message box with an information string.

@param infoString	The information string to display.

@param infoNumber       Not used

*/
void UploaderGadgetWidget::info(QString infoString, int infoNumber)
{
    status->setText(infoString);
}
