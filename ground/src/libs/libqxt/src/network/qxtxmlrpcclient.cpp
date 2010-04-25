/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtWeb module of the Qxt library.
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
/*!
    \class QxtXmlRpcClient
    \inmodule QxtNetwork
    \brief The QxtXmlRpcClient class implements an XML-RPC Client.

    Implements a Client that can communicate with services implementing the XML-RPC spec
    http://www.xmlrpc.com/spec

    \section2 Type Conversion

    \table 80%
    \header \o XML-RPC Type \o Qt Type
    \row  \o array \o QVariantList
    \row  \o base64 \o QByteArray   (decoded from base64 to raw)
    \row  \o boolean \o bool
    \row  \o dateTime \o QDateTime
    \row  \o double \o double
    \row  \o int \o int
    \row  \o i4 \o int
    \row  \o string \o QString
    \row  \o struct \o QVariantMap
    \row  \o nil \o QVariant()
    \endtable

*/

#include "qxtxmlrpcclient.h"
#include "qxtxmlrpccall.h"
#include "qxtxmlrpc_p.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

struct QxtXmlRpcClient::Private
{
    QUrl url;
    QNetworkAccessManager * networkManager;
    void serialize(QVariant);
};

QxtXmlRpcClient::QxtXmlRpcClient(QObject * parent)
        : QObject(parent)
        , d(new Private())
{
    d->networkManager = new QNetworkAccessManager(this);
}

/*!
  returns the url of the remote service
 */
QUrl QxtXmlRpcClient::serviceUrl() const
{
    return d->url;
}

/*!
  sets the url of the remote service to \a url
 */
void QxtXmlRpcClient::setServiceUrl(QUrl url)
{
    d->url = url;
}
/*!
  returns the QNetworkAccessManager used for connecting to the remote service
 */
QNetworkAccessManager * QxtXmlRpcClient::networkManager() const
{
    return d->networkManager;
}
/*!
  sets the QNetworkAccessManager used for connecting to the remote service to \a manager
 */

void QxtXmlRpcClient::setNetworkManager(QNetworkAccessManager * manager)
{
    delete d->networkManager;
    d->networkManager = manager;
}

/*!
  calls the remote \a method with \a arguments and returns a QxtXmlRpcCall wrapping it.
  you can connect to QxtXmlRpcCall's signals to retreive the status of the call.
 */
QxtXmlRpcCall * QxtXmlRpcClient::call(QString method, QVariantList arguments)
{
    QByteArray data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><methodCall><methodName>" + method.toUtf8() + "</methodName><params>";
    foreach(QVariant i, arguments)
    {
        data += "<param>" + QxtXmlRpc::serialize(i).toUtf8() + "</param>";
    }
    data += "</params></methodCall>";


    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml");
    request.setRawHeader("Connection", "close");
    request.setUrl(d->url);

    return new QxtXmlRpcCall(d->networkManager->post(request, data));
}
