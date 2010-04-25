/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtCore module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/

#include "qxtbasicfileloggerengine.h"
#include <QDateTime>

/*!
    \class QxtBasicFileLoggerEngine
    \brief The QxtBasicFileLoggerEngine class provides a basic file logger engine.
    \inmodule QxtCore

    Example basic file log output:
    \code
    [22:38:33.159] [Error] Unknown error
    [22:51:43.488] [Debug] What's going on?
                           Hi there!
    \endcode

    \sa QxtLogger
 */

class QxtBasicFileLoggerEnginePrivate : public QxtPrivate<QxtBasicFileLoggerEngine>
{
public:
    QXT_DECLARE_PUBLIC(QxtBasicFileLoggerEngine)
    QString dateFormat;
};

/*!
    Constructs a basic file logger engine with \a fileName.
*/
QxtBasicFileLoggerEngine::QxtBasicFileLoggerEngine(const QString &fileName)
        : QxtAbstractFileLoggerEngine(fileName, QIODevice::ReadWrite | QIODevice::Append | QIODevice::Unbuffered)
{
    QXT_INIT_PRIVATE(QxtBasicFileLoggerEngine);
    qxt_d().dateFormat = "hh:mm:ss.zzz";
}

/*!
    Returns the date format in use by this logger engine.
    \sa QDateTime::toString()
 */
QString QxtBasicFileLoggerEngine::dateFormat() const
{
    return qxt_d().dateFormat;
}

/*!
    Sets the date \a format used by this logger engine.
    \sa QDateTime::toString()
 */
void QxtBasicFileLoggerEngine::setDateFormat(const QString& format)
{
    qxt_d().dateFormat = format;
}

/*!
    \reimp
 */
void QxtBasicFileLoggerEngine::writeToFile(const QString &level, const QVariantList &messages)
{
    if (messages.isEmpty()) return;
    QString header = '[' + QDateTime::currentDateTime().toString(qxt_d().dateFormat) + "] [" + level + "] ";
    QString padding;
    QIODevice* file = device();
    Q_ASSERT(file);
    file->write(header.toUtf8());
    for (int i = 0; i < header.size(); i++) padding.append(" ");
    int count = 0;
    Q_FOREACH(const QVariant& out, messages)
    {
        if (!out.isNull())
        {
            if (count != 0) file->write(padding.toUtf8());
            file->write(out.toString().toUtf8());
            file->write("\n");
        }
        count++;
    }
}
