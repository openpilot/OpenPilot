/**
 ******************************************************************************
 *
 * @file       tst_pluginmanager.cpp
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

#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginspec.h>
#include <extensionsystem/iplugin.h>

#include <QtTest/QtTest>

#include <QtCore/QObject>

using namespace ExtensionSystem;

class SignalReceiver;

class tst_PluginManager : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void addRemoveObjects();
    void getObject();
    void getObjects();
    void plugins();
    void circularPlugins();
    void correctPlugins1();

private:
    PluginManager *m_pm;
    SignalReceiver *m_sr;
};

class SignalReceiver : public QObject
{
    Q_OBJECT

public:
    SignalReceiver() :
        objectAddedCount(0),
        aboutToRemoveObjectCount(0),
        pluginsChangedCount(0),
        objectAddedObj(0),
        aboutToRemoveObjectObj(0)
    { }
    int objectAddedCount;
    int aboutToRemoveObjectCount;
    int pluginsChangedCount;
    QObject *objectAddedObj;
    QObject *aboutToRemoveObjectObj;
public slots:
    void objectAdded(QObject *obj) { objectAddedCount++; objectAddedObj = obj; }
    void aboutToRemoveObject(QObject *obj) { aboutToRemoveObjectCount++; aboutToRemoveObjectObj = obj; }
    void pluginsChanged() { pluginsChangedCount++; }
};

class MyClass1 : public QObject
{
    Q_OBJECT
};

class MyClass2 : public QObject
{
    Q_OBJECT
};

class MyClass11 : public MyClass1
{
    Q_OBJECT
};

void tst_PluginManager::init()
{
    m_pm = new PluginManager;
    m_sr = new SignalReceiver;
    connect(m_pm, SIGNAL(objectAdded(QObject*)), m_sr, SLOT(objectAdded(QObject*)));
    connect(m_pm, SIGNAL(aboutToRemoveObject(QObject*)), m_sr, SLOT(aboutToRemoveObject(QObject*)));
    connect(m_pm, SIGNAL(pluginsChanged()), m_sr, SLOT(pluginsChanged()));
}

void tst_PluginManager::cleanup()
{
    delete m_pm;
    delete m_sr;
}

void tst_PluginManager::addRemoveObjects()
{
    QObject *object1 = new QObject;
    QObject *object2 = new QObject;
    QCOMPARE(m_pm->allObjects().size(), 0);
    m_pm->addObject(object1);
    QCOMPARE(m_sr->objectAddedCount, 1);
    QCOMPARE(m_sr->objectAddedObj, object1);
    QCOMPARE(m_sr->aboutToRemoveObjectCount, 0);
    QVERIFY(m_pm->allObjects().contains(object1));
    QVERIFY(!m_pm->allObjects().contains(object2));
    QCOMPARE(m_pm->allObjects().size(), 1);
    m_pm->addObject(object2);
    QCOMPARE(m_sr->objectAddedCount, 2);
    QCOMPARE(m_sr->objectAddedObj, object2);
    QCOMPARE(m_sr->aboutToRemoveObjectCount, 0);
    QVERIFY(m_pm->allObjects().contains(object1));
    QVERIFY(m_pm->allObjects().contains(object2));
    QCOMPARE(m_pm->allObjects().size(), 2);
    m_pm->removeObject(object1);
    QCOMPARE(m_sr->objectAddedCount, 2);
    QCOMPARE(m_sr->aboutToRemoveObjectCount, 1);
    QCOMPARE(m_sr->aboutToRemoveObjectObj, object1);
    QVERIFY(!m_pm->allObjects().contains(object1));
    QVERIFY(m_pm->allObjects().contains(object2));
    QCOMPARE(m_pm->allObjects().size(), 1);
    m_pm->removeObject(object2);
    QCOMPARE(m_sr->objectAddedCount, 2);
    QCOMPARE(m_sr->aboutToRemoveObjectCount, 2);
    QCOMPARE(m_sr->aboutToRemoveObjectObj, object2);
    QVERIFY(!m_pm->allObjects().contains(object1));
    QVERIFY(!m_pm->allObjects().contains(object2));
    QCOMPARE(m_pm->allObjects().size(), 0);
    delete object1;
    delete object2;
}

void tst_PluginManager::getObject()
{
    MyClass2 *object2 = new MyClass2;
    MyClass11 *object11 = new MyClass11;
    m_pm->addObject(object2);
    QCOMPARE(m_pm->getObject<MyClass11>(), (MyClass11*)0);
    QCOMPARE(m_pm->getObject<MyClass1>(), (MyClass1*)0);
    QCOMPARE(m_pm->getObject<MyClass2>(), object2);
    m_pm->addObject(object11);
    QCOMPARE(m_pm->getObject<MyClass11>(), object11);
    QCOMPARE(m_pm->getObject<MyClass1>(), qobject_cast<MyClass1*>(object11));
    QCOMPARE(m_pm->getObject<MyClass2>(), object2);
    m_pm->removeObject(object2);
    m_pm->removeObject(object11);
    delete object2;
    delete object11;
}

void tst_PluginManager::getObjects()
{
    MyClass1 *object1 = new MyClass1;
    MyClass2 *object2 = new MyClass2;
    MyClass11 *object11 = new MyClass11;
    m_pm->addObject(object2);
    QCOMPARE(m_pm->getObjects<MyClass11>(), QList<MyClass11*>());
    QCOMPARE(m_pm->getObjects<MyClass1>(), QList<MyClass1*>());
    QCOMPARE(m_pm->getObjects<MyClass2>(), QList<MyClass2*>() << object2);
    QCOMPARE(m_pm->allObjects(), QList<QObject*>() << object2);
    m_pm->addObject(object11);
    QCOMPARE(m_pm->getObjects<MyClass11>(), QList<MyClass11*>() << object11);
    QCOMPARE(m_pm->getObjects<MyClass1>(), QList<MyClass1*>() << object11);
    QCOMPARE(m_pm->getObjects<MyClass2>(), QList<MyClass2*>() << object2);
    QCOMPARE(m_pm->allObjects(), QList<QObject*>() << object2 << object11);
    m_pm->addObject(object1);
    QCOMPARE(m_pm->getObjects<MyClass11>(), QList<MyClass11*>() << object11);
    QCOMPARE(m_pm->getObjects<MyClass1>(), QList<MyClass1*>() << object11 << object1);
    QCOMPARE(m_pm->getObjects<MyClass2>(), QList<MyClass2*>() << object2);
    QCOMPARE(m_pm->allObjects(), QList<QObject*>() << object2 << object11 << object1);
    m_pm->removeObject(object2);
    m_pm->removeObject(object11);
    m_pm->removeObject(object1);
    delete object1;
    delete object2;
    delete object11;
}

void tst_PluginManager::plugins()
{
    m_pm->setPluginPaths(QStringList() << "plugins");
    QCOMPARE(m_sr->pluginsChangedCount, 1);
    QSet<PluginSpec *> plugins = m_pm->plugins();
    QCOMPARE(plugins.count(), 3);
    foreach (const QString &expected, QStringList() << "helloworld" << "MyPlugin" << "dummyPlugin") {
        bool found = false;
        foreach (PluginSpec *spec, plugins) {
            if (spec->name() == expected) {
                found = true;
                break;
            }
        }
        QVERIFY2(found, QString("plugin '%1' not found").arg(expected).toLocal8Bit().constData());
    }
}

void tst_PluginManager::circularPlugins()
{
    m_pm->setPluginPaths(QStringList() << "circularplugins");
    m_pm->loadPlugins();
    foreach (PluginSpec *spec, m_pm->plugins()) {
        if (spec->name() == "plugin1") {
            QVERIFY(spec->hasError());
            QCOMPARE(spec->state(), PluginSpec::Resolved);
            QCOMPARE(spec->plugin(), (IPlugin*)0);
        } else if (spec->name() == "plugin2") {
            QVERIFY(!spec->hasError());
            QCOMPARE(spec->state(), PluginSpec::Running);
        } else if (spec->name() == "plugin3") {
            QVERIFY(spec->hasError());
            QCOMPARE(spec->state(), PluginSpec::Resolved);
            QCOMPARE(spec->plugin(), (IPlugin*)0);
        }
    }
}

void tst_PluginManager::correctPlugins1()
{
    m_pm->setFileExtension("spec");
    m_pm->setPluginPaths(QStringList() << "correctplugins1");
    m_pm->loadPlugins();
    foreach (PluginSpec *spec, m_pm->plugins()) {
        if (spec->hasError())
            qDebug() << spec->errorString();
        QVERIFY(!spec->hasError());
        QCOMPARE(spec->state(), PluginSpec::Running);
    }
    bool plugin1running = false;
    bool plugin2running = false;
    bool plugin3running = false;
    foreach (QObject *obj, m_pm->allObjects()) {
        if (obj->objectName() == "MyPlugin1_running")
            plugin1running = true;
        else if (obj->objectName() == "MyPlugin2_running")
            plugin2running = true;
        else if (obj->objectName() == "MyPlugin3_running")
            plugin3running = true;
    }
    QVERIFY(plugin1running);
    QVERIFY(plugin2running);
    QVERIFY(plugin3running);
}

QTEST_MAIN(tst_PluginManager)

#include "tst_pluginmanager.moc"

