/**
 ******************************************************************************
 *
 * @file       texteditloggerengine.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Qxt Foundation http://www.libqxt.org Copyright (C)
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConsolePlugin Console Plugin
 * @{
 * @brief The Console Gadget impliments a console view 
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

#include "texteditloggerengine.h"
#include <QTime>
#include <QtGui/QTextEdit>
#include <QtGui/QScrollBar>
#include <QObject>

#define QXT_REQUIRED_LEVELS (QxtLogger::WarningLevel | QxtLogger::ErrorLevel | QxtLogger::CriticalLevel | QxtLogger::FatalLevel)

TextEditLoggerEngine::TextEditLoggerEngine(QTextEdit *textEdit) : m_textEdit(textEdit)
{
#ifndef QT_NO_DEBUG
    setLogLevelsEnabled(QXT_REQUIRED_LEVELS);
#else
    setLogLevelsEnabled(QXT_REQUIRED_LEVELS | QxtLogger::DebugLevel);
#endif
    enableLogging();
}

TextEditLoggerEngine::~TextEditLoggerEngine()
{
}

void TextEditLoggerEngine::initLoggerEngine()
{
    return; // Should work out of the box!
}

void TextEditLoggerEngine::killLoggerEngine()
{
    return; // I do nothing.
}

bool TextEditLoggerEngine::isInitialized() const
{
    return true;
}

void TextEditLoggerEngine::setLogLevelEnabled(QxtLogger::LogLevels level, bool enable)
{
    QxtLoggerEngine::setLogLevelsEnabled(level | QXT_REQUIRED_LEVELS, enable);
    if (!enable) QxtLoggerEngine::setLogLevelsEnabled(QXT_REQUIRED_LEVELS);
}

void TextEditLoggerEngine::writeFormatted(QxtLogger::LogLevel level, const QList<QVariant> &msgs)
{
    switch (level)
    {
    case QxtLogger::ErrorLevel:
        writeToTextEdit("Error", msgs, Qt::red);
        break;
    case QxtLogger::WarningLevel:
        writeToTextEdit("Warning", msgs, Qt::red);
        break;
    case QxtLogger::CriticalLevel:
        writeToTextEdit("Critical", msgs, Qt::red);
        break;
    case QxtLogger::FatalLevel:
        writeToTextEdit("!!FATAL!!", msgs, Qt::red);
        break;
    case QxtLogger::TraceLevel:
        writeToTextEdit("Trace", msgs, Qt::blue);
        break;
    case QxtLogger::DebugLevel:
        writeToTextEdit("DEBUG", msgs, Qt::blue);
        break;
    case QxtLogger::InfoLevel:
        writeToTextEdit("INFO", msgs);
        break;
    default:
        writeToTextEdit("", msgs);
        break;
    }
}

void TextEditLoggerEngine::writeToTextEdit(const QString& level, const QList<QVariant> &msgs, QColor color)
{
    /* Message format...
        [time] [error level] First message.....
                    second message
                    third message
    */
    if (msgs.isEmpty())
        return;
    QScrollBar *sb = m_textEdit->verticalScrollBar();
    bool scroll = sb->value() == sb->maximum();
    QString header = '[' + QTime::currentTime().toString("hh:mm:ss.zzz") + "] [" + level + "] ";
    QString padding;
    QString appendText;
    appendText.append(header);
    for (int i = 0; i < header.size(); i++) padding.append(' ');
    int count = 0;
    Q_FOREACH(const QVariant& out, msgs)
    {
        if (!out.isNull())
        {
            if (count != 0)
                appendText.append(padding);
            appendText.append(out.toString());
        }
        count++;
    }
    Q_ASSERT(m_textEdit);
    appendText = QString("<font color=%1>%2</font>").arg(color.name()).arg(appendText);
    m_textEdit->append(appendText);
    if (scroll)
        sb->setValue(sb->maximum());
}
