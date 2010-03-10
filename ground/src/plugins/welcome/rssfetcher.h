/**
 ******************************************************************************
 *
 * @file       rssfetcher.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   
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

#ifndef RSSFETCHER_H
#define RSSFETCHER_H

#include <QtCore/QUrl>
#include <QtCore/QXmlStreamReader>
#include <QtNetwork/QHttp>

namespace Welcome {
namespace Internal {

class RSSFetcher : public QObject
{
    Q_OBJECT
public:
    RSSFetcher(int maxItems, QObject *parent = 0);

signals:
    void newsItemReady(const QString& title, const QString& desciption, const QString& url);

public slots:
    void fetch(const QUrl &url);
    void finished(int id, bool error);
    void readData(const QHttpResponseHeader &);

 signals:
    void finished(bool error);

private:
    void parseXml();

    QXmlStreamReader m_xml;
    QString m_currentTag;
    QString m_linkString;
    QString m_descriptionString;
    QString m_titleString;

    QHttp m_http;
    int m_connectionId;
    int m_items;
    int m_maxItems;
};

} // namespace Welcome
} // namespace Internal

#endif // RSSFETCHER_H

