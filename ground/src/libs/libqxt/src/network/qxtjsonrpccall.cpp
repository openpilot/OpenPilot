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
    \class QxtJSONRpcCall
    \inmodule QxtNetwork
    \brief The QxtJSONRpcCall class represents a Call to an JSON-RPC Service.

*/

/*!
    \fn QxtJSONRpcCall::downloadProgress ( qint64 bytesReceived, qint64 bytesTotal )

    This signal is emitted to indicate the progress of retreiving the response from the remote service

    \sa QNetworkReply::downloadProgress()
*/

/*!
    \fn QxtJSONRpcCall::error(QNetworkReply::NetworkError code)

    Signals a network error

    \sa QNetworkReply::error()
*/

/*!
    \fn QxtJSONRpcCall::finished()

    emitted once the result is fully available

    \sa QNetworkReply::finished()
*/

/*!
    \fn QxtJSONRpcCall::sslErrors ( const QList<QSslError> & errors );


    \sa QNetworkReply::sslErrors()
*/

/*!
    \fn QxtJSONRpcCall:: uploadProgress()

    This signal is emitted to indicate the progress of sending the request to the remote service

    \sa QNetworkReply::uploadProgress()
*/

#include "qxtjsonrpccall.h"
#include <QXmlStreamReader>
#include <QNetworkReply>
#include <QxtJSON>

class QxtJSONRpcCallPrivate
{
public:
    bool isFault;
    QNetworkReply * reply;
    QVariant result;
    void d_finished();
    QxtJSONRpcCall * pub;
};


/*!
  returns true if the remote service sent a fault message
*/
bool QxtJSONRpcCall::isFault() const
{
    return d->isFault;
}

/*!
  returns the result or fault message or a null QVariant() if the call isnt finished yet

  \sa QxtJSON#type-conversion
*/
QVariant QxtJSONRpcCall::result() const
{
    return d->result;
}

/*!
  returns an associated network error.
*/
QNetworkReply::NetworkError QxtJSONRpcCall::error() const
{
    return d->reply->error();
}

QxtJSONRpcCall::QxtJSONRpcCall(QNetworkReply * reply)
        : d(new QxtJSONRpcCallPrivate())
{
    d->isFault = false;
    d->reply = reply;
    d->pub = this;
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SIGNAL(downloadProgress(qint64, qint64)));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SIGNAL(error(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(sslErrors(const QList<QSslError> &)), this, SIGNAL(sslErrors(const QList<QSslError> &)));
    connect(reply, SIGNAL(uploadProgress(qint64, qint64)), this, SIGNAL(uploadProgress(qint64, qint64)));
    connect(reply, SIGNAL(finished()), this, SLOT(d_finished()));
}

void QxtJSONRpcCallPrivate::d_finished()
{
    if (!reply->error())
    {
        QVariant m_=QxtJSON::parse(QString::fromUtf8(reply->readAll()));
        if(m_.isNull()){
            qWarning("QxtJSONRpcCall: invalid JSON received");
        }

        QVariantMap m=m_.toMap();

        if(m["error"]!=QVariant()){
            isFault=true;
            result=m["error"];
        }else{
            result=m["result"];
        }
    }
    emit pub->finished();
}


#include "moc_qxtjsonrpccall.cpp"
