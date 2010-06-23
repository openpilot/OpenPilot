/**
 ******************************************************************************
 *
 * @file       rssfetcher.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   welcomeplugin
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

#include <QtCore/QDebug>
#include <QtCore/QSysInfo>
#include <QtCore/QLocale>
#include <QtGui/QDesktopServices>
#include <QtGui/QLineEdit>
#include <QtNetwork/QHttp>
#include <QtNetwork/QNetworkProxyFactory>

#include <coreplugin/coreconstants.h>

#include "rssfetcher.h"

#ifdef Q_OS_UNIX
#include <sys/utsname.h>
#endif

using namespace Welcome::Internal;

static const QString getOsString()
{
    QString osString;
#if defined(Q_OS_WIN)
    switch (QSysInfo::WindowsVersion) {
    case (QSysInfo::WV_4_0):
        osString += QLatin1String("WinNT4.0");
        break;
    case (QSysInfo::WV_5_0):
        osString += QLatin1String("Windows NT 5.0");
        break;
    case (QSysInfo::WV_5_1):
        osString += QLatin1String("Windows NT 5.1");
        break;
    case (QSysInfo::WV_5_2):
        osString += QLatin1String("Windows NT 5.2");
        break;
    case (QSysInfo::WV_6_0):
        osString += QLatin1String("Windows NT 6.0");
        break;
    case (QSysInfo::WV_6_1):
        osString += QLatin1String("Windows NT 6.1");
        break;
    default:
        osString += QLatin1String("Windows NT (Unknown)");
        break;
    }
#elif defined (Q_OS_MAC)
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
        osString += QLatin1String("PPC ");
    else
        osString += QLatin1String("Intel ");
    osString += QLatin1String("Mac OS X ");
    switch (QSysInfo::MacintoshVersion) {
    case (QSysInfo::MV_10_3):
        osString += QLatin1String("10_3");
        break;
    case (QSysInfo::MV_10_4):
        osString += QLatin1String("10_4");
        break;
    case (QSysInfo::MV_10_5):
        osString += QLatin1String("10_5");
        break;
    case (QSysInfo::MV_10_6):
        osString += QLatin1String("10_6");
        break;
    default:
        osString += QLatin1String("(Unknown)");
        break;
    }
#elif defined (Q_OS_UNIX)
    struct utsname uts;
    if (uname(&uts) == 0)
        osString += QString("%1 %2").arg(QLatin1String(uts.sysname))
                        .arg(QLatin1String(uts.release));
    else
        osString += QLatin1String("Unix (Unknown)");
#else
    ossttring = QLatin1String("Unknown OS");
#endif
    return osString;
}

RSSFetcher::RSSFetcher(int maxItems, QObject *parent)
    : QObject(parent), m_items(0), m_maxItems(maxItems)
{
    connect(&m_http, SIGNAL(readyRead(const QHttpResponseHeader &)),
             this, SLOT(readData(const QHttpResponseHeader &)));

    connect(&m_http, SIGNAL(requestFinished(int, bool)),
             this, SLOT(finished(int, bool)));
}

void RSSFetcher::fetch(const QUrl &url)
{
    QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(QNetworkProxyQuery(url));
    if (proxies.count() > 0)
        m_http.setProxy(proxies.first());
    m_http.setHost(url.host());
    QString agentStr = QString("OP-GCS/%1 (QHttp %2; %3; %4; %5 bit)")
                    .arg(Core::Constants::GCS_VERSION_LONG).arg(qVersion())
                    .arg(getOsString()).arg(QLocale::system().name())
                    .arg(QSysInfo::WordSize);
    QHttpRequestHeader header("GET", url.path());
    //qDebug() << agentStr;
    header.setValue("User-Agent", agentStr);
    header.setValue("Host", url.host());
    m_connectionId = m_http.request(header);
}

void RSSFetcher::readData(const QHttpResponseHeader &resp)
{
    if (resp.statusCode() != 200)
        m_http.abort();
    else {
        m_xml.addData(m_http.readAll());
        parseXml();
    }
}

void RSSFetcher::finished(int id, bool error)
{
    Q_UNUSED(id)
    m_items = 0;
    emit finished(error);
}

void RSSFetcher::parseXml()
{
    while (!m_xml.atEnd()) {
        m_xml.readNext();
        if (m_xml.isStartElement()) {
            if (m_xml.name() == "item") {
                m_titleString.clear();
                m_descriptionString.clear();
                m_linkString.clear();
            }
            m_currentTag = m_xml.name().toString();
        } else if (m_xml.isEndElement()) {
            if (m_xml.name() == "item") {
                m_items++;
                if (m_items > m_maxItems)
                    return;
                emit newsItemReady(m_titleString, m_descriptionString, m_linkString);
            }

        } else if (m_xml.isCharacters() && !m_xml.isWhitespace()) {
            if (m_currentTag == "title")
                m_titleString += m_xml.text().toString();
            else if (m_currentTag == "description")
                m_descriptionString += m_xml.text().toString();
            else if (m_currentTag == "link")
                m_linkString += m_xml.text().toString();
        }
    }
    if (m_xml.error() && m_xml.error() != QXmlStreamReader::PrematureEndOfDocumentError) {
        qWarning() << "XML ERROR:" << m_xml.lineNumber() << ": " << m_xml.errorString();
        m_http.abort();
    }
}
