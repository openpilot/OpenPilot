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
    \class QxtJSONRpcClient
    \inmodule QxtNetwork
    \brief The QxtJSONRpcClient class implements a JSON-RPC Client.

    Implements a Client that can communicate with services implementing the JSON-RPC spec
    http://json-rpc.org/wiki/specification

    \sa QxtJSON

*/

#include "qxtjsonrpcclient.h"
#include "qxtjsonrpccall.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QxtJSON>

struct QxtJSONRpcClient::Private
{
    int callid;
    QUrl url;
    QNetworkAccessManager * networkManager;
};

QxtJSONRpcClient::QxtJSONRpcClient(QObject * parent)
        : QObject(parent)
        , d(new Private())
{
    d->callid=0;
    d->networkManager = new QNetworkAccessManager(this);
}

/*!
  returns the url of the remote service
 */
QUrl QxtJSONRpcClient::serviceUrl() const
{
    return d->url;
}

/*!
  sets the url of the remote service to \a url
 */
void QxtJSONRpcClient::setServiceUrl(QUrl url)
{
    d->url = url;
}
/*!
  returns the QNetworkAccessManager used for connecting to the remote service
 */
QNetworkAccessManager * QxtJSONRpcClient::networkManager() const
{
    return d->networkManager;
}
/*!
  sets the QNetworkAccessManager used for connecting to the remote service to \a manager
 */

void QxtJSONRpcClient::setNetworkManager(QNetworkAccessManager * manager)
{
    delete d->networkManager;
    d->networkManager = manager;
}

/*!
  calls the remote \a method with \a arguments and returns a QxtJSONRpcCall wrapping it.
  you can connect to QxtJSONRpcCall's signals to retreive the status of the call.
 */
QxtJSONRpcCall * QxtJSONRpcClient::call(QString method, QVariantList arguments)
{
    QVariantMap m;
    m["id"]=d->callid++;
    m["method"]=method;
    m["params"]=arguments;

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain; charset=utf-8");
    request.setRawHeader("Connection", "close");
    request.setUrl(d->url);

    return new QxtJSONRpcCall(d->networkManager->post(request, QxtJSON::stringify(m).toUtf8()));
}
