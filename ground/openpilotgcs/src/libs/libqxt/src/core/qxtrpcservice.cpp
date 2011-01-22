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

#include "qxtrpcservice.h"
#include "qxtrpcservice_p.h"
#include "qxtabstractconnectionmanager.h"
#include "qxtdatastreamsignalserializer.h"
#include "qxtmetaobject.h"
#include "qxtboundfunction.h"
#include <QIODevice>
#include <QtDebug>
#include <QMetaObject>
#include <QMetaType>
#include <QMetaMethod>
#include <QMultiHash>
#include <QList>
#include <QString>
#include <QByteArray>
#include <QPair>

/*!
 * \class QxtRPCService
 * \inmodule QxtCore
 * \brief The QxtRPCService class transmits Qt signals over a QIODevice
 *
 * QxtRPCService is a tool that encapsulates Qt signals and transmits them over a QIODevice. The signal is subsequently 
 * re-emitted on the receiving end of the connection.
 *
 * QxtRPCService can act as a client or a server. When acting as a server, it uses a QxtAbstractConnectionManager to 
 * accept connections. When acting as a client, the application should provide an already-connected QIODevice to the
 * setDevice() function.
 *
 * All data types used in attached signals and slots must be declared and registered with QMetaType using
 * Q_DECLARE_METATYPE and qRegisterMetaType. Additional requirements may be imposed by the QxtAbstractSignalSerializer
 * subclass in use; the default (QxtDataStreamSignalSerializer) requires that they have stream operators registered
 * with qRegisterMetaTypeStreamOperators.
 *
 * Due to a restriction of Qt's signals and slots mechanism, the number of parameters that can be passed to call() and
 * its related functions, as well as the number of parameters to any signal or slot attached to QxtRPCService, is
 * limited to 8.
 */

/*
 * QxtRPCServiceIntrospector is a customized QObject subclass that implements dynamic slots instead of using a static
 * metaobject. Trolltech/Nokia has published a document on the concept in Qt Quarterly, available at
 * <http://doc.trolltech.com/qq/qq16-dynamicqobject.html>, although work on QxtRPCService (originally QxRPCPeer) had
 * begun before this document was released to non-subscribers.
 *
 * This class is responsible for dealing with incoming Qt signals.
 */
class QxtRPCServiceIntrospector : public QObject
{
public:
    // Maintain a pointer back to the service object that owns the introspector.
    QxtRPCService* rpc;

    // Keep track of the ID of the next virtual slot to be defined.
    quint32 nextSlotID;

    // Maps an incoming signal's metamethod to an entry in signalParameters.
    QHash<QxtRPCServicePrivate::MetaMethodDef, int> signalIDs;

    // Maps an incoming signal's metamethod to its index in its metaobject.
    QHash<QxtRPCServicePrivate::MetaMethodDef, int> methodIDs;

    // Each signal and slot has a list of parameters associated with it.
    // QMetaType provides a type ID for every recognized data type.
    QList<QList<int> > signalParameters;

    // A signal connection can be identified by the object emitting it and the signature of the signal.
    typedef QPair<QObject*, QByteArray> SignalDef;

    // Maps a signal connection to one or more dynamic slot IDs. 
    QMultiHash<SignalDef, int> signalToId;

    // Maps a dynamic slot ID to an entry in signalParameters.
    QHash<int, int> idToParams;

    // Keep track of which RPC messages should be emitted for each dynamically created slot.
    QMultiHash<int, QString> idToRpc;

    QxtRPCServiceIntrospector(QxtRPCService* parent);
    bool addSignal(QObject* obj, const char* signature, const QString& rpcFunction);
    void disconnectObject(QObject* obj);
    int qt_metacall(QMetaObject::Call _c, int _id, void **_a);
};

QxtRPCServiceIntrospector::QxtRPCServiceIntrospector(QxtRPCService* parent)
: QObject(parent), rpc(parent)
{
    // We start numbering our dynamic slots one after the last static one in the superclass.
    nextSlotID = QObject::staticMetaObject.methodCount();
}

bool QxtRPCServiceIntrospector::addSignal(QObject* obj, const char* signature, const QString& rpcFunction)
{
    const QMetaObject* meta = obj->metaObject();
    QByteArray norm = QxtMetaObject::methodSignature(signature);
    QxtRPCServicePrivate::MetaMethodDef signal = qMakePair(meta, norm);
    int sigID, methodID;

    if(!signalIDs.count(signal)) {
        // This signal hasn't been encountered before, so read the metaobject and cache the results.
        methodID = meta->indexOfMethod(norm.constData());
        if(methodID < 0) {
            // indexOfMethod() returns -1 if the signal was not found, so report a warning and return an error.
            qWarning() << "QxtRPCService::attachSignal: " << obj << "::" << signature << " does not exist";
            return false;
        }

        // Look up the signal's parameter list, ensure each parameter is queueable, and cache the type IDs.
        QList<QByteArray> types = meta->method(methodID).parameterTypes();
        QList<int> typeIDs;
        int ct = types.count();
        for(int i = 0; i < ct; i++) {
            int typeID = QMetaType::type(types.value(i).constData());
            if(typeID <= 0) {
                qWarning() << "QxtRPCService::attachSignal: cannot queue arguments of type " << types.value(i);
                return false;
            }
            typeIDs.append(typeID);
        }

        // Cache the looked-up parameter list, associate the list with the signal definition, and cache the signal's
        // QMetaObject method index.
        int nextSignalID = signalParameters.count();
        signalParameters.append(typeIDs);
        signalIDs[signal] = nextSignalID;
        methodIDs[signal] = methodID;

        // And finally, set the signal ID for the connect call below.
        sigID = nextSignalID;
    } else {
        // Use the cached values from before.
        sigID = signalIDs.value(signal);
        methodID = methodIDs.value(signal);
    }

    // Use an undocumented function from QMetaObject to connect the incoming signal to the next dynamic slot.
    bool success = QMetaObject::connect(obj, methodID, this, nextSlotID);
    if(!success) {
        // Presumedly connect() will output its own warning.
        // The remaining setup is unnecessary if the connection failed.
        return false; 
    }

    // Associate the signal and source object with the dynamic slot.
    signalToId.insert(qMakePair(obj, norm), nextSlotID);

    // Associate the new dynamic slot with an RPC message, or use the signal signature as the default message.
    idToRpc.insert(nextSlotID, rpcFunction.isEmpty() ? norm : rpcFunction);

    // Associate the new dynamic slot with the parameter list definition.
    idToParams[nextSlotID] = sigID;

    do {
        // This variable is perfectly allowed to wrap around; it's terribly unlikely to have four billion
        // concurrent connections but a particularly long-lived object with a lot of disconnects and
        // reconnects might survive to see a wraparound.
        nextSlotID++;
        // But of course we need to make sure the next ID isn't in use.
    } while(nextSlotID < quint32(QObject::staticMetaObject.methodCount()) || idToParams.contains(nextSlotID));

    return true;
}

void QxtRPCServiceIntrospector::disconnectObject(QObject* obj)
{
    const QMetaObject* meta = obj->metaObject();

    foreach(const SignalDef& sig, signalToId.keys()) {
        // Iterate through all tracked connections and skip any that don't match the incoming object.
        if(sig.first != obj) continue;

        int methodID = methodIDs[qMakePair(meta, sig.second)];
        foreach(int id, signalToId.values(sig)) {
            // Iterate through all of the different connections for the object and explicitly disconnect them.
            QMetaObject::disconnect(obj, methodID, this, id);

            // Remove the connection from our mappings.
            idToRpc.remove(id);
            idToParams.remove(id);
        }

        // Remove the object/signal from the mapping.
        signalToId.remove(sig);
    }
}

int QxtRPCServiceIntrospector::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    // Qt's signal dispatch mechanism invokes qt_metacall for each slot connected to the object.

    // The metacall protocol expects slots to be checked from the bottom up, starting with QObject and proceeding to
    // the more derived classes. qt_metacall returns a negative number to indicate that the request was processed or a
    // positive number to indicate the greatest method ID that was checked. moc-generated qt_metacall implementations
    // subtract the return value from _id so that slots on a given class can be counted starting at 0, allowing the
    // subclasses to add new signals or slots without breaking compatibility. QxtRPCService doesn't need this because
    // it just asks its base class how many methods it has before adding slots.
    if(QObject::qt_metacall(_c, _id, _a) < 0)
        return _id;

    // qt_metacall is also used for other behaviors besides just invoking methods; we don't implement any of these, so
    // we just return here.
    if(_c != QMetaObject::InvokeMetaMethod)
        return _id;

    // Construct an array of QVariants based on the parameters passed through _a.
    QVariant v[8];
    const QList<int>& types = signalParameters.at(idToParams.value(_id));
    int ct = types.count();
    for(int i = 0; i < ct; i++) {
        // The qt_metacall implementation is expected to already know the data types in _a, so that's why we tracked it.
        v[i] = QVariant(types.at(i), _a[i+1]);
    }

    foreach(const QString& rpcName, idToRpc.values(_id)) {
        // Invoke each RPC message connected to the requested dynamic slot ID.
        rpc->call(rpcName, v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
    }

    // Inform the calling function that we handled the call.
    return -1;
}

QxtRPCServicePrivate::QxtRPCServicePrivate()
: QObject(NULL), manager(NULL), serializer(new QxtDataStreamSignalSerializer), device(NULL)
{
    // initializers only
    // As you can see, the default serializer is a QxtDataStreamSerializer.
}

void QxtRPCServicePrivate::clientConnected(QIODevice* dev, quint64 id)
{
    // QxtMetaObject::bind() is a nice piece of magic that allows parameters to a slot to be defined in the connection.
    QxtMetaObject::connect(dev, SIGNAL(readyRead()),
                           QxtMetaObject::bind(this, SLOT(clientData(quint64)), Q_ARG(quint64, id)));

    // Inform other objects that a new client has connected.
    emit qxt_p().clientConnected(id);

    // Initialize a new buffer for this connection.
    buffers[id] = QByteArray();

    // If there's any unread data in the device, go ahead and process it up front.
    if(dev->bytesAvailable() > 0)
        clientData(id);
}

void QxtRPCServicePrivate::clientDisconnected(QIODevice* dev, quint64 id)
{
    // When a device is disconnected, disconnect all signals connected to the object...
    QObject::disconnect(dev, 0, this, 0);
    QObject::disconnect(dev, 0, &qxt_p(), 0);

    // ... remove its buffer object...
    buffers.remove(id);

    // ... and inform other objects that the disconnection has happened.
    emit qxt_p().clientDisconnected(id);
}

void QxtRPCServicePrivate::clientData(quint64 id)
{
    // Get the device from the connection manager.
    QIODevice* dev = manager->client(id);

    // Cache a reference to the buffer.
    QByteArray& buf = buffers[id]; 

    // Read all available data on the device.
    buf.append(dev->readAll());

    while(serializer->canDeserialize(buf)) {
        // Extract one deserialized signal from the buffer.
        QxtAbstractSignalSerializer::DeserializedData data = serializer->deserialize(buf);

        // Check to see if it's a blank command.
        if(serializer->isNoOp(data))
            continue;

        // Check for protocol errors.
        if(serializer->isProtocolError(data)) {
            qWarning() << "QxtRPCService: Invalid data received; disconnecting";
            qxt_p().disconnectClient(id);
            return;
        }

        // Pad the arguments to 8, because that's what dispatchFromClient() expects.
        while(data.second.count() < 8)
            data.second << QVariant();

        // And finally, invoke the dispatcher.
        dispatchFromClient(id, data.first, data.second[0], data.second[1], data.second[2], data.second[3],
                data.second[4], data.second[5], data.second[6], data.second[7]);
    }
}

void QxtRPCServicePrivate::serverData()
{
    // This function does the same thing as clientData() except there's only one server connection instead of
    // multiple client connections.

    // Read all available data on the device.
    serverBuffer.append(device->readAll());

    while(serializer->canDeserialize(serverBuffer)) {
        // Extract one deserialized signal from the buffer.
        QxtAbstractSignalSerializer::DeserializedData data = serializer->deserialize(serverBuffer);

        // Check to see if it's a blank command.
        if(serializer->isNoOp(data))
            continue;

        // Check for protocol errors.
        if(serializer->isProtocolError(data)) {
            qWarning() << "QxtRPCService: Invalid data received; disconnecting";
            qxt_p().disconnectServer();
            return;
        }

        // Pad the arguments to 8, because that's what dispatchFromServer() expects.
        while(data.second.count() < 8)
            data.second << QVariant();

        // And finally, invoke the dispatcher.
        dispatchFromServer(data.first, data.second[0], data.second[1], data.second[2], data.second[3], data.second[4],
                data.second[5], data.second[6], data.second[7]);
    }
}

// Constructing a QGenericArgument object is generally done with a Q_ARG macro; this macro is a convenience that
// does basically the same thing but dramatically shortens the call to invokeMethod().
#define QXT_ARG(i) ((numParams>i)?QGenericArgument(p ## i .typeName(), p ## i.constData()):QGenericArgument())

void QxtRPCServicePrivate::dispatchFromServer(const QString& fn, const QVariant& p0, const QVariant& p1,
        const QVariant& p2, const QVariant& p3, const QVariant& p4, const QVariant& p5, const QVariant& p6,
        const QVariant& p7) const
{
    // If the received message is not connected to any slots, ignore it.
    if(!connectedSlots.contains(fn)) return;

    foreach(const SlotDef& slot, connectedSlots.value(fn)) {
        // Look up the parameters for each slot based on its metamethod definition.
        MetaMethodDef method = qMakePair(slot.recv->metaObject(), slot.slot);
        const QList<QByteArray>& params = slotParameters.value(method);
        int numParams = params.count();

        // Invoke the specified slot on the receiver object using the arguments passed to the function. The
        // QGenericArgument stuff is done here for safety, as it's not inconceivable (but it IS dangerous) for
        // different slots to have different parameter lists.
        if(!QMetaObject::invokeMethod(slot.recv, slot.slot.constData(), slot.type, QXT_ARG(0), QXT_ARG(1), QXT_ARG(2),
                    QXT_ARG(3), QXT_ARG(4), QXT_ARG(5), QXT_ARG(6), QXT_ARG(7))) {
            qWarning() << "QxtRPCService: invokeMethod for " << slot.recv << "::" << slot.slot << " failed";
        }
    }
}

void QxtRPCServicePrivate::dispatchFromClient(quint64 id, const QString& fn, const QVariant& p0, const QVariant& p1,
        const QVariant& p2, const QVariant& p3, const QVariant& p4, const QVariant& p5, const QVariant& p6,
        const QVariant& p7) const
{
    // If the received message is not connected to any slots, ignore it.
    if(!connectedSlots.contains(fn)) return; 

    foreach(const SlotDef& slot, connectedSlots.value(fn))
    {
        // Look up the parameters for each slot based on its metamethod definition.
        MetaMethodDef method = qMakePair(slot.recv->metaObject(), slot.slot);
        const QList<QByteArray>& params = slotParameters.value(method);
        int numParams = params.count();

        // Invoke the specified slot on the receiver object using the arguments passed to the function.
        // See dispatchFromServer() for a discussion of the safety of QXT_ARG here.
        if(!QMetaObject::invokeMethod(slot.recv, slot.slot.constData(), slot.type, Q_ARG(quint64, id), QXT_ARG(0),
                    QXT_ARG(1), QXT_ARG(2), QXT_ARG(3), QXT_ARG(4), QXT_ARG(5), QXT_ARG(6), QXT_ARG(7))) {
            qWarning() << "QxtRPCService: invokeMethod for " << slot.recv << "::" << slot.slot << " failed";
        }
    }
}

/*!
 * Creates a QxtRPCService object with the given \a parent.
 */
QxtRPCService::QxtRPCService(QObject* parent) : QObject(parent)
{
    // Every QxtRPCService object has two private worker objects.
    // QxtRPCServicePrivate is responsible for most of the heavy lifting.
    QXT_INIT_PRIVATE(QxtRPCService);
    // QxtRPCServiceIntrospector is responsible for capturing and processing incoming signals.
    qxt_d().introspector = new QxtRPCServiceIntrospector(this);
}

/*!
 * Creates a QxtRPCService object with the given \a parent and connects it to the specified I/O \a device.
 *
 * The I/O device must already be opened for reading and writing.
 */
QxtRPCService::QxtRPCService(QIODevice* device, QObject* parent) : QObject(parent)
{
    QXT_INIT_PRIVATE(QxtRPCService);
    qxt_d().introspector = new QxtRPCServiceIntrospector(this);
    setDevice(device);
}

/*!
 * Destroys the QxtRPCService object.
 */
QxtRPCService::~QxtRPCService()
{
    // QxtAbstractSignalSerializer isn't a QObject, so we have to explicitly clean it up.
    delete qxt_d().serializer;
}

/*!
 * Returns \c true if the connection manager is accepting connections or if any clients are currently connected. It is
 * possible for both isServer() and isClient() to return \c false if the connection manager is not accepting
 * connections, no clients are connected, and no QIODevice is set for a server.
 * \sa isClient()
 */
bool QxtRPCService::isServer() const
{
    return qxt_d().manager && (qxt_d().manager->isAcceptingConnections() || qxt_d().manager->clientCount() > 0);
}

/*!
 * Returns \c true if the QxtRPCService is currently communicating with a server. It is possible for both isServer()
 * and isClient() to return \c false if the connection manager is not accepting connections, no clients are connected,
 * and no QIODevice is set for a server.
 * \sa isServer()
 */
bool QxtRPCService::isClient() const
{
    return qxt_d().device != NULL;
}

/*!
 * Disconnects a client with \a id using the attached connection manager.
 *
 * If connected to a server, this function is ignored with a warning.
 */
void QxtRPCService::disconnectClient(quint64 id)
{
    if(!isServer()) {
        qWarning() << "QxtRPCService::disconnectClient: not operating as a server";
        return;
    }

    if(!qxt_d().manager->client(id)) {
        qWarning() << "QxtRPCService::disconnectClient: no client with specified ID";
        return;
    }

    // Ask the manager to disconnect the client. QxtAbstractConnectionManager will emit disconnected(), which is chained
    // to QxtRPCService::clientDisconnected(), so that signal is not explicitly emitted here.
    qxt_d().manager->disconnect(id);
}

/*!
 * Disconnects from the server. The QIODevice is deleted. Use takeDevice() to disconnect from the server without
 * deleting the device.
 *
 * If not connected to a server, for instance if acting as a server, this function is ignored with a warning.
 * \sa device()
 * \sa takeDevice()
 */
void QxtRPCService::disconnectServer()
{
    if(!isClient()) {
        qWarning() << "QxtRPCService::disconnectServer: not connected to a server";
        return;
    }
    takeDevice()->deleteLater();
}

/*!
 * Disconnects all clients, or disconnects from the server.
 */
void QxtRPCService::disconnectAll()
{
    if(isClient()) 
        disconnectServer();

    if(isServer()) {
        QList<quint64> clientIDs = clients();
        foreach(quint64 id, clientIDs) {
            // Disconnect from each client in turn.
            disconnectClient(id);
        }
    }
}

/*!
 * Returns a list of client IDs for all connected clients.
 */
QList<quint64> QxtRPCService::clients() const
{
    if(!isServer()) {
        qWarning() << "QxtRPCService::clients: not a server";
        return QList<quint64>();
    }

    // This function is mostly a convenience wrapper.
    return qxt_d().manager->clients();
}

/*!
 * When operating as a client, returns the QIODevice connected to the server.
 * When operating as a server, or if not connected to a server, returns NULL.
 * \sa setDevice()
 * \sa takeDevice()
 */
QIODevice* QxtRPCService::device() const
{
    return qxt_d().device;
}

/*!
 * Begins communicating with a server through the provided QIODevice \a dev. If called while acting as a server, this
 * function is ignored with a warning. If called while another device is set, the original QIODevice is deleted. The
 * provided device will be reparented to the QxtRPCService.
 *
 * Note that because QIODevice is a generic interface, QxtRPCService cannot provide signals when this device is
 * disconnected or has low-level errors. Connect to the QIODevice subclass's signals directly if you need this
 * information.
 * \sa device()
 */
void QxtRPCService::setDevice(QIODevice* dev)
{
    // First, delete the old device if one is set.
    if(qxt_d().device)
        delete qxt_d().device;

    // Then set the device and claim ownership of it.
    qxt_d().device = dev;
    dev->setParent(this);

    // Listen for data arriving on the device.
    QObject::connect(dev, SIGNAL(readyRead()), &qxt_d(), SLOT(serverData()));

    // If there's already data available on the device, process it.
    if(dev->bytesAvailable() > 0)
        qxt_d().serverData();
}

/*!
 * When operating as a client, returns the QIODevice used to communicate with the server. After this function is called,
 * the QxtRPCService will no longer be connected and device() will return NULL. When operating as a server, or if not
 * connected to a server, this function returns NULL without any other effect.
 * \sa device()
 */
QIODevice* QxtRPCService::takeDevice()
{
    QIODevice* oldDevice = qxt_d().device;
    if(oldDevice) {
        // Make sure all signals from the device are disconnected before releasing it so that we don't get spurious
        // signals firing off where we don't want them.
        QObject::disconnect(oldDevice, 0, this, 0);
        QObject::disconnect(oldDevice, 0, &qxt_d(), 0);
        qxt_d().device = NULL;
    }
    return oldDevice;
}

/*!
 * Returns the signal serializer used to encode signals before transmission.
 * \sa setSerializer()
 */
QxtAbstractSignalSerializer* QxtRPCService::serializer() const
{
    return qxt_d().serializer;
}

/*!
 * Sets the signal \a serializer used to encode signals before transmission. The existing serializer will be deleted.
 * \sa serializer()
 */
void QxtRPCService::setSerializer(QxtAbstractSignalSerializer* serializer)
{
    delete qxt_d().serializer;
    qxt_d().serializer = serializer;
}

/*!
 * Returns the connection manager used to accept incoming connections.
 * \sa setConnectionManager()
 */
QxtAbstractConnectionManager* QxtRPCService::connectionManager() const
{
    return qxt_d().manager;
}

/*!
 * Sets the connection \a manager used to accept incoming connections. The existing manager will be deleted and the
 * provided manager will be reparented to the QxtRPCService.
 * \sa connectionManager()
 */
void QxtRPCService::setConnectionManager(QxtAbstractConnectionManager* manager)
{
    // Delete the old manager, if one is set.
    if(qxt_d().manager)
        delete qxt_d().manager;

    // Set the manager and claim ownership of it. 
    qxt_d().manager = manager;
    manager->setParent(this);

    // Listen for connections and disconnections.
    QObject::connect(manager, SIGNAL(newConnection(QIODevice*, quint64)),
                     &qxt_d(), SLOT(clientConnected(QIODevice*, quint64)));
    QObject::connect(manager, SIGNAL(disconnected(QIODevice*, quint64)),
                     &qxt_d(), SLOT(clientDisconnected(QIODevice*, quint64)));
}

/*!
 * Attaches the given signal.
 *
 * When the attached \a signal is emitted by \a sender, it will be transmitted to all connected servers, clients, or
 * peers. If an optional \a rpcFunction is provided, it will be used in place of the name of the transmitted signal.
 * Use the SIGNAL() macro to specify the signal, just as you would for QObject::connect().
 *
 * Like QObject::connect(), attachSignal() returns \c false if the connection cannot be established.
 */
bool QxtRPCService::attachSignal(QObject* sender, const char* signal, const QString& rpcFunction)
{
    return qxt_d().introspector->addSignal(sender, signal, rpcFunction);
}

/*!
 * Attaches an RPC function to the given slot.
 *
 * When a signal with the name given by \a rpcFunction is received from the network, the attached \a slot is executed.
 * Use the SLOT() macro to specify the slot, just as you would for QObject::connect().
 *
 * Like QObject::connect(), attachSlot() returns \c false if the connection cannot be established.
 * Also like QObject::connect(), a signal may be used as a slot; invocation will cause the signal to be emitted.
 *
 * \bold {Note:} When acting like a server, the first parameter of the slot must be <b>quint64 id</b>. The parameters 
 * of the incoming signal follow. For example, SIGNAL(mySignal(QString)) from the client connects to
 * SLOT(mySlot(quint64, QString)) on the server.
 */
bool QxtRPCService::attachSlot(const QString& rpcFunction, QObject* recv, const char* slot, Qt::ConnectionType type)
{
    const QMetaObject* meta = recv->metaObject();
    QByteArray name = QxtMetaObject::methodName(slot);
    QxtRPCServicePrivate::MetaMethodDef info = qMakePair(meta, name);

    if(!qxt_d().slotParameters.count(info)) {
        // This method hasn't been encountered before, so read the metaobject and cache the results.
        QByteArray norm = QxtMetaObject::methodSignature(slot);
        int methodID = meta->indexOfMethod(norm.constData());
        if(methodID < 0) {
            // indexOfMethod() returns -1 if the method was not found, so report a warning and return an error.
            qWarning() << "QxtRPCService::attachSlot: " << recv << "::" << slot << " does not exist";
            return false;
        }

        // Look up the method's parameter list, ensure each parameter is queueable, and cache the type IDs.
        QList<QByteArray> types = meta->method(methodID).parameterTypes();
        int ct = types.count();
        for(int i = 0; i < ct; i++) {
            int typeID = QMetaType::type(types.value(i).constData());
            if(typeID <= 0) {
                qWarning() << "QxtRPCService::attachSlot: cannot queue arguments of type " << types.value(i);
                return false;
            }
        }
        
        // Cache the looked-up parameter list.
        qxt_d().slotParameters[info] = types;
    }

    // If the RPC function name appears to be a signal or slot, normalize the signature.
    QString rpcFunc = rpcFunction;
    if(QxtMetaObject::isSignalOrSlot(rpcFunction.toAscii().constData()))
        rpcFunc = QxtMetaObject::methodSignature(rpcFunction.toAscii().constData());

    // Construct a slot definition and associate it with the RPC function name.
    QxtRPCServicePrivate::SlotDef slotDef;
    slotDef.recv = recv;
    slotDef.slot = name;
    slotDef.type = type;
    qxt_d().connectedSlots[rpcFunc].append(slotDef);

    return true;
}

/*!
 * Detaches all signals and slots for the given object \a obj.
 */
void QxtRPCService::detachObject(QObject* obj)
{
    detachSignals(obj);
    detachSlots(obj);
}

/*!
 * Detaches all signals for the given object \a obj.
 */
void QxtRPCService::detachSignals(QObject* obj)
{
    qxt_d().introspector->disconnectObject(obj);
}

/*!
* Detaches all slots for the given object \a obj.
 */
void QxtRPCService::detachSlots(QObject* obj)
{
    foreach(const QString& name, qxt_d().connectedSlots.keys()) {
        // Iterate over all connected slots.
        foreach(const QxtRPCServicePrivate::SlotDef& slot, qxt_d().connectedSlots.value(name)) {
            // Skip slots on other objects.
            if(slot.recv != obj) continue;
            qxt_d().connectedSlots[name].removeAll(slot);
        }
    }
}

/*!
 * Sends the signal \a fn with the given parameter list to the server, or to all connected clients.
 *
 * The receiver is not obligated to act upon the signal. If no clients are connected, and if not communicating with a
 * server, this function does nothing.
 */
void QxtRPCService::call(QString fn, const QVariant& p1, const QVariant& p2, const QVariant& p3, const QVariant& p4,
                         const QVariant& p5, const QVariant& p6, const QVariant& p7, const QVariant& p8)
{
    if(isClient()) {
        // Normalize the function name if it has the form of a signal or slot.
        if(QxtMetaObject::isSignalOrSlot(fn.toAscii().constData()))
            fn = QxtMetaObject::methodSignature(fn.toAscii().constData());

        // Serialize the parameters and write the result to the device.
        QByteArray data = qxt_d().serializer->serialize(fn, p1, p2, p3, p4, p5, p6, p7, p8);
        qxt_d().device->write(data);
    }

    if(isServer()) {
        // Delegate the call to the other overload of call().
        call(clients(), fn, p1, p2, p3, p4, p5, p6, p7, p8);
    }
}

/*!
 * Sends the signal \a fn with the given parameter list to the provided list of clients.
 *
 * The receivers are not obligated to act upon the signal. If no client is connected with a provided ID, the ID is
 * ignored with a warning. If acting as a client, this function does nothing.
 */
void QxtRPCService::call(QList<quint64> ids, QString fn, const QVariant& p1, const QVariant& p2, const QVariant& p3,
        const QVariant& p4, const QVariant& p5, const QVariant& p6, const QVariant& p7, const QVariant& p8)
{
    // Serialize the parameters first.
    QByteArray data = qxt_d().serializer->serialize(fn, p1, p2, p3, p4, p5, p6, p7, p8);

    foreach(quint64 id, ids) {
        // Find the specified client.
        QIODevice* dev = qxt_d().manager->client(id);
        if(!dev) {
            qWarning() << "QxtRPCService::call: client ID not connected";
            continue;
        }

        // Transmit the data to the client.
        dev->write(data);
    }
}

/*!
 * Sends the signal \a fn with the given parameter list to the specified client.
 *
 * The receiver is not obligated to act upon the signal. If no client with the given ID is connected, the call will be
 * ignored with a warning. If acting as a client, this function does nothing.
 */
void QxtRPCService::call(quint64 id, QString fn, const QVariant& p1, const QVariant& p2, const QVariant& p3,
        const QVariant& p4, const QVariant& p5, const QVariant& p6, const QVariant& p7, const QVariant& p8)
{
    // Wrap the ID in a list and delegate it to the appropriate overload.
    call(QList<quint64>() << id, fn, p1, p2, p3, p4, p5, p6, p7, p8);
}

/*!
 * Sends the signal \a fn with the given parameter list to all connected clients except for the client specified.
 *
 * The receiver is not obligated to act upon the signal. This function is useful for rebroadcasting a signal from one
 * client to all other connected clients. If acting as a client, this function does nothing.
 */
void QxtRPCService::callExcept(quint64 id, QString fn, const QVariant& p1, const QVariant& p2, const QVariant& p3,
        const QVariant& p4, const QVariant& p5, const QVariant& p6, const QVariant& p7, const QVariant& p8)
{
    // Get the list of clients and remove the exception, then delegate the call to the appropriate overload of call().
    QList<quint64> ids = clients();
    ids.removeAll(id);
    call(ids, fn, p1, p2, p3, p4, p5, p6, p7, p8);
}

/*!
 * Detaches all signals and slots for the object that emitted the signal connected to detachSender().
 */
void QxtRPCService::detachSender()
{
    detachObject(sender());
}

