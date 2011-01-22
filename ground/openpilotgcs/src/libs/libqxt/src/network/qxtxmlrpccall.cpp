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
    \class QxtXmlRpcCall
    \inmodule QxtNetwork
    \brief The QxtXmlRpcCall class represents a Call to an XMLRPC Service.

*/

/*!
    \fn QxtXmlRpcCall::downloadProgress ( qint64 bytesReceived, qint64 bytesTotal )

    This signal is emitted to indicate the progress of retreiving the response from the remote service

    \sa QNetworkReply::downloadProgress()
*/

/*!
    \fn QxtXmlRpcCall::error(QNetworkReply::NetworkError code)

    Signals a network error

    \sa QNetworkReply::error()
*/

/*!
    \fn QxtXmlRpcCall::finished()

    emitted once the result is fully available

    \sa QNetworkReply::finished()
*/

/*!
    \fn QxtXmlRpcCall::sslErrors ( const QList<QSslError> & errors );


    \sa QNetworkReply::sslErrors()
*/

/*!
    \fn QxtXmlRpcCall:: uploadProgress()

    This signal is emitted to indicate the progress of sending the request to the remote service

    \sa QNetworkReply::uploadProgress()
*/

#include "qxtxmlrpccall.h"
#include "qxtxmlrpc_p.h"
#include <QXmlStreamReader>
#include <QNetworkReply>

class QxtXmlRpcCallPrivate
{
public:
    bool isFault;
    QNetworkReply * reply;
    QVariant result;
    void d_finished();
    QxtXmlRpcCall * pub;
};


/*!
  returns true if the remote service sent a fault message
*/
bool QxtXmlRpcCall::isFault() const
{
    return d->isFault;
}

/*!
  returns the result or fault message or a null QVariant() if the call isnt finished yet

  \sa QxtXmlRpcClient#type-conversion
*/
QVariant QxtXmlRpcCall::result() const
{
    return d->result;
}

/*!
  returns an associated network error.
*/
QNetworkReply::NetworkError QxtXmlRpcCall::error() const
{
    return d->reply->error();
}

QxtXmlRpcCall::QxtXmlRpcCall(QNetworkReply * reply)
        : d(new QxtXmlRpcCallPrivate())
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

void QxtXmlRpcCallPrivate::d_finished()
{

    if (!reply->error())
    {
        int s = 0;

        QXmlStreamReader xml(reply->readAll());
        while (!xml.atEnd())
        {
            xml.readNext();
            if (xml.isStartElement())
            {
                if (s == 0)
                {
                    if (xml.name().toString() == "methodResponse")
                    {
                        s = 1;
                    }
                    else
                    {
                        xml.raiseError("expected <methodResponse>,  got:<" + xml.name().toString() + ">");
                    }
                }
                else if (s == 1)
                {
                    if (xml.name().toString() == "params")
                    {
                        s = 2;
                    }
                    else if (xml.name().toString() == "fault")
                    {
                        isFault = true;
                        s = 3;
                    }
                    else
                    {
                        xml.raiseError("expected <params> or <fault>,  got:<" + xml.name().toString() + ">");
                    }
                }
                else if (s == 2)
                {
                    if (xml.name().toString() == "param")
                    {
                        s = 3;
                    }
                    else
                    {
                        xml.raiseError("expected <param>,  got:<" + xml.name().toString() + ">");
                    }
                }
                else if (s == 3)
                {
                    if (xml.name().toString() == "value")
                    {
                        result = QxtXmlRpc::deserialize(xml);
                        s = 4;
                    }
                    else
                    {
                        xml.raiseError("expected <value>,  got:<" + xml.name().toString() + ">");
                    }
                }

            }
        }
        if (xml.hasError())
        {
            qWarning("QxtXmlRpcCall: %s at line %lld column %lld", xml.errorString().toLocal8Bit().data(),
                     xml.lineNumber(),
                     xml.columnNumber());
        }
    }
    emit pub->finished();
}


#include "moc_qxtxmlrpccall.cpp"
