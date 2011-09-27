/**
 ******************************************************************************
 *
 * @file       debuggadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup DebugGadgetPlugin Debug Gadget Plugin
 * @{
 * @brief A place holder gadget plugin 
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
#include "debuggadgetwidget.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include "qxtlogger.h"
#include "debugengine.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QTime>
DebugGadgetWidget::DebugGadgetWidget(QWidget *parent) : QLabel(parent)
{
    m_config = new Ui_Form();
    m_config->setupUi(this);
    debugengine * de=new debugengine();
    QxtLogger::getInstance()->addLoggerEngine("debugplugin", de);
    connect(de,SIGNAL(dbgMsg(QString,QList<QVariant>)),this,SLOT(dbgMsg(QString,QList<QVariant>)));
    connect(de,SIGNAL(dbgMsgError(QString,QList<QVariant>)),this,SLOT(dbgMsgError(QString,QList<QVariant>)));
    connect(m_config->pushButton,SIGNAL(clicked()),this,SLOT(saveLog()));
}

DebugGadgetWidget::~DebugGadgetWidget()
{
    // Do nothing
}

void DebugGadgetWidget::dbgMsg(const QString &level, const QList<QVariant> &msgs)
{
    m_config->plainTextEdit->setTextColor(Qt::red);

        m_config->plainTextEdit->append(QString("%2[%0]%1").arg(level).arg(msgs[0].toString()).arg(QTime::currentTime().toString()));

    QScrollBar *sb = m_config->plainTextEdit->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void DebugGadgetWidget::dbgMsgError(const QString &level, const QList<QVariant> &msgs)
{
    m_config->plainTextEdit->setTextColor(Qt::black);


        m_config->plainTextEdit->append(QString("%2[%0]%1").arg(level).arg(msgs[0].toString()).arg(QTime::currentTime().toString()));

    QScrollBar *sb = m_config->plainTextEdit->verticalScrollBar();
    sb->setValue(sb->maximum());
}
void DebugGadgetWidget::saveLog()
{
    QString fileName = QFileDialog::getSaveFileName(0, tr("Save log File As"), "");
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly) &&
            (file.write(m_config->plainTextEdit->toHtml().toAscii()) != -1)) {
        file.close();
    } else {
        QMessageBox::critical(0,
                              tr("Log Save"),
                              tr("Unable to save log: ") + fileName,
                              QMessageBox::Ok);
        return;
    }
}
