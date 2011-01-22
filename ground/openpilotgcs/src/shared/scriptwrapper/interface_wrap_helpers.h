/**
 ******************************************************************************
 *
 * @file       interface_wrap_helpers.h
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

#ifndef INTERFACE_WRAP_HELPERS_H
#define INTERFACE_WRAP_HELPERS_H

#include <QtScript/QScriptEngine>

namespace SharedTools {

// Convert a QObjectInterface to Scriptvalue
// To be registered as a magic creation function with qScriptRegisterMetaType().
// (see registerQObjectInterface)

template <class QObjectInterface>
static QScriptValue qObjectInterfaceToScriptValue(QScriptEngine *engine, QObjectInterface* const &qoif)
{
    if (!qoif)
        return  QScriptValue(engine, QScriptValue::NullValue);

    QObject *qObject = const_cast<QObjectInterface *>(qoif);

    const QScriptEngine::QObjectWrapOptions wrapOptions =
        QScriptEngine::ExcludeChildObjects|QScriptEngine::ExcludeSuperClassMethods|QScriptEngine::ExcludeSuperClassProperties;
    return engine->newQObject(qObject, QScriptEngine::QtOwnership, wrapOptions);
}

// Convert  Scriptvalue back to  QObjectInterface
// To be registered as a magic conversion function with  qScriptRegisterMetaType().
// (see registerQObjectInterface)

template <class QObjectInterface>
static void scriptValueToQObjectInterface(const QScriptValue &sv, QObjectInterface *&p)
{
    QObject *qObject =  sv.toQObject();
    p = qobject_cast<QObjectInterface*>(qObject);
}

// Magically register a Workbench interface derived from
// ExtensionSystem::QObjectInterface class with the engine.
// To avoid lifecycle issues, the script value is created on the QObject returned
// by ExtensionSystem::QObjectInterface::qObject() and given the specified
// prototype. By convention, ExtensionSystem::QObjectInterface::qObject() returns an
// QObject that implements the interface, so it can be casted to it.

template <class QObjectInterface, class Prototype>
static void registerQObjectInterface(QScriptEngine *engine)
{
    Prototype *protoType = new Prototype(engine);
    const QScriptValue scriptProtoType = engine->newQObject(protoType);

    const int metaTypeId = qScriptRegisterMetaType<QObjectInterface*>(
        engine,
        qObjectInterfaceToScriptValue<QObjectInterface>,
        scriptValueToQObjectInterface<QObjectInterface>,
        scriptProtoType);
    Q_UNUSED(metaTypeId)
}

} // namespace SharedTools

#endif // INTERFACE_WRAP_HELPERS_H
