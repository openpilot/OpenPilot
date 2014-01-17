/**
 ******************************************************************************
 *
 * @file       configtaskwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectWidgetUtils Plugin
 * @{
 * @brief Utility plugin for UAVObject to Widget relation management
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
#include "configtaskwidget.h"
#include <QWidget>
#include <QLineEdit>
#include "uavsettingsimportexport/uavsettingsimportexportfactory.h"

ConfigTaskWidget::ConfigTaskWidget(QWidget *parent) : QWidget(parent), m_isConnected(false), m_isWidgetUpdatesAllowed(true),
    m_saveButton(NULL), m_isDirty(false), m_outOfLimitsStyle("background-color: rgb(255, 0, 0);"), m_realtimeUpdateTimer(NULL)
{
    m_pluginManager     = ExtensionSystem::PluginManager::instance();
    TelemetryManager *telMngr = m_pluginManager->getObject<TelemetryManager>();
    m_objectUtilManager = m_pluginManager->getObject<UAVObjectUtilManager>();
    connect(telMngr, SIGNAL(connected()), this, SLOT(onAutopilotConnect()), Qt::UniqueConnection);
    connect(telMngr, SIGNAL(disconnected()), this, SLOT(onAutopilotDisconnect()), Qt::UniqueConnection);
    connect(telMngr, SIGNAL(connected()), this, SIGNAL(autoPilotConnected()), Qt::UniqueConnection);
    connect(telMngr, SIGNAL(disconnected()), this, SIGNAL(autoPilotDisconnected()), Qt::UniqueConnection);
    UAVSettingsImportExportFactory *importexportplugin = m_pluginManager->getObject<UAVSettingsImportExportFactory>();
    connect(importexportplugin, SIGNAL(importAboutToBegin()), this, SLOT(invalidateObjects()));
}

void ConfigTaskWidget::addWidget(QWidget *widget)
{
    addWidgetBinding("", "", widget);
}

void ConfigTaskWidget::addUAVObject(QString objectName, QList<int> *reloadGroups)
{
    addWidgetBinding(objectName, "", NULL, 0, 1, false, reloadGroups);
}

void ConfigTaskWidget::addUAVObject(UAVObject *objectName, QList<int> *reloadGroups)
{
    addUAVObject(objectName ? objectName->getName() : QString(""), reloadGroups);
}

int ConfigTaskWidget::fieldIndexFromElementName(QString objectName, QString fieldName, QString elementName)
{
    if (elementName.isEmpty() || objectName.isEmpty()) {
        return 0;
    }

    QString singleObjectName = mapObjectName(objectName).split(",").at(0);
    UAVObject *object     = getObject(singleObjectName);
    Q_ASSERT(object);

    UAVObjectField *field = object->getField(fieldName);
    Q_ASSERT(field);

    return field->getElementNames().indexOf(elementName);
}

void ConfigTaskWidget::addWidgetBinding(QString objectName, QString fieldName, QWidget *widget, QString elementName)
{
    addWidgetBinding(objectName, fieldName, widget, fieldIndexFromElementName(objectName, fieldName, elementName));
}

void ConfigTaskWidget::addWidgetBinding(UAVObject *object, UAVObjectField *field, QWidget *widget, QString elementName)
{
    addWidgetBinding(object ? object->getName() : QString(""), field ? field->getName() : QString(""), widget, elementName);
}

void ConfigTaskWidget::addWidgetBinding(QString objectName, QString fieldName, QWidget *widget, QString elementName, double scale,
                                        bool isLimited, QList<int> *reloadGroupIDs, quint32 instID)
{
    addWidgetBinding(objectName, fieldName, widget, fieldIndexFromElementName(objectName, fieldName, elementName),
                     scale, isLimited, reloadGroupIDs, instID);
}

void ConfigTaskWidget::addWidgetBinding(UAVObject *object, UAVObjectField *field, QWidget *widget, QString elementName, double scale,
                                        bool isLimited, QList<int> *reloadGroupIDs, quint32 instID)
{
    addWidgetBinding(object ? object->getName() : QString(""), field ? field->getName() : QString(""), widget, elementName, scale,
                     isLimited, reloadGroupIDs, instID);
}

void ConfigTaskWidget::addWidgetBinding(UAVObject *object, UAVObjectField *field, QWidget *widget, int index, double scale,
                                        bool isLimited, QList<int> *reloadGroupIDs, quint32 instID)
{
    addWidgetBinding(object ? object->getName() : QString(""), field ? field->getName() : QString(""), widget, index, scale,
                     isLimited, reloadGroupIDs, instID);
}

void ConfigTaskWidget::addWidgetBinding(QString objectName, QString fieldName, QWidget *widget, int index, double scale,
                                        bool isLimited, QList<int> *reloadGroupIDs, quint32 instID)
{
    QString mappedObjectName = mapObjectName(objectName);

    // If object name is comma separated list of objects, call one time per objectName
    foreach(QString singleObjectName, mappedObjectName.split(",")) {
        doAddWidgetBinding(singleObjectName, fieldName, widget, index, scale, isLimited, reloadGroupIDs, instID);
    }
}

void ConfigTaskWidget::doAddWidgetBinding(QString objectName, QString fieldName, QWidget *widget, int index, double scale,
                                          bool isLimited, QList<int> *reloadGroupIDs, quint32 instID)
{
    if (addShadowWidgetBinding(objectName, fieldName, widget, index, scale, isLimited, reloadGroupIDs, instID)) {
        return;
    }

    UAVObject *object     = NULL;
    UAVObjectField *field = NULL;
    if (!objectName.isEmpty()) {
        object = getObject(QString(objectName), instID);
        Q_ASSERT(object);
        m_updatedObjects.insert(object, true);
        connect(object, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(objectUpdated(UAVObject *)));
        connect(object, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(refreshWidgetsValues(UAVObject *)), Qt::UniqueConnection);
    }

    if (!fieldName.isEmpty() && object) {
        field = object->getField(QString(fieldName));
    }

    WidgetBinding *binding = new WidgetBinding(widget, object, field, index, scale, isLimited);
    // Only the first binding per widget can be enabled.
    binding->setIsEnabled(m_widgetBindingsPerWidget.count(widget) == 0);
    m_widgetBindingsPerWidget.insert(widget, binding);


    if (object) {
        m_widgetBindingsPerObject.insert(object, binding);
        if (m_saveButton) {
            m_saveButton->addObject((UAVDataObject *)object);
        }
    }

    if (!widget) {
        if (reloadGroupIDs && object) {
            foreach(int groupId, *reloadGroupIDs) {
                m_reloadGroups.insert(groupId, binding);
            }
        }
    } else {
        connectWidgetUpdatesToSlot(widget, SLOT(widgetsContentsChanged()));
        if (reloadGroupIDs) {
            addWidgetToReloadGroups(widget, reloadGroupIDs);
        }
        if (binding->isEnabled()) {
            loadWidgetLimits(widget, field, index, isLimited, scale);
        }
    }
}

void ConfigTaskWidget::setWidgetBindingObjectEnabled(QString objectName, bool enabled)
{
    UAVObject *object = getObject(objectName);

    Q_ASSERT(object);

    bool dirtyBack = isDirty();

    foreach(WidgetBinding * binding, m_widgetBindingsPerObject.values(object)) {
        binding->setIsEnabled(enabled);
        if (enabled) {
            if (binding->value().isValid() && !binding->value().isNull()) {
                setWidgetFromVariant(binding->widget(), binding->value(), binding->scale());
            } else {
                setWidgetFromField(binding->widget(), binding->field(), binding->index(), binding->scale(), binding->isLimited());
            }
        }
    }
    setDirty(dirtyBack);
}

ConfigTaskWidget::~ConfigTaskWidget()
{
    if (m_saveButton) {
        delete m_saveButton;
    }
    QSet<WidgetBinding *> deleteSet = m_widgetBindingsPerWidget.values().toSet();
    foreach(WidgetBinding * binding, deleteSet) {
        if (binding) {
            delete binding;
        }
    }
    if (m_realtimeUpdateTimer) {
        delete m_realtimeUpdateTimer;
        m_realtimeUpdateTimer = NULL;
    }
}

void ConfigTaskWidget::saveObjectToSD(UAVObject *obj)
{
    // saveObjectToSD is now handled by the UAVUtils plugin in one
    // central place (and one central queue)
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectUtilManager *utilMngr     = pm->getObject<UAVObjectUtilManager>();

    utilMngr->saveObjectToSD(obj);
}

UAVObjectManager *ConfigTaskWidget::getObjectManager()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objMngr = pm->getObject<UAVObjectManager>();

    Q_ASSERT(objMngr);
    return objMngr;
}

double ConfigTaskWidget::listMean(QList<double> list)
{
    double accum = 0;

    for (int i = 0; i < list.size(); i++) {
        accum += list[i];
    }
    return accum / list.size();
}

double ConfigTaskWidget::listVar(QList<double> list)
{
    double mean_accum = 0;
    double var_accum  = 0;
    double mean;

    for (int i = 0; i < list.size(); i++) {
        mean_accum += list[i];
    }
    mean = mean_accum / list.size();

    for (int i = 0; i < list.size(); i++) {
        var_accum += (list[i] - mean) * (list[i] - mean);
    }

    // Use unbiased estimator
    return var_accum / (list.size() - 1);
}

void ConfigTaskWidget::onAutopilotDisconnect()
{
    m_isConnected = false;
    enableControls(false);
    invalidateObjects();
}

void ConfigTaskWidget::forceConnectedState() // dynamic widgets don't recieve the connected signal. This should be called instead.
{
    m_isConnected = true;
    setDirty(false);
}

void ConfigTaskWidget::onAutopilotConnect()
{
    if (m_objectUtilManager) {
        m_currentBoardId = m_objectUtilManager->getBoardModel(); // TODO REMEMBER TO ADD THIS TO FORCE CONNECTED FUNC ON CC3D_RELEASE
    }
    invalidateObjects();
    m_isConnected = true;
    foreach(WidgetBinding * binding, m_widgetBindingsPerObject) {
        if (!binding->isEnabled()) {
            continue;
        }
        loadWidgetLimits(binding->widget(), binding->field(), binding->index(), binding->isLimited(), binding->scale());
    }
    setDirty(false);
    enableControls(true);
    refreshWidgetsValues();
}

void ConfigTaskWidget::populateWidgets()
{
    bool dirtyBack = isDirty();
    emit populateWidgetsRequested();

    foreach(WidgetBinding * binding, m_widgetBindingsPerObject) {
        if (binding->isEnabled() && binding->object() != NULL && binding->field() != NULL && binding->widget() != NULL) {
            setWidgetFromField(binding->widget(), binding->field(), binding->index(), binding->scale(), binding->isLimited());
        }
    }
    setDirty(dirtyBack);
}

void ConfigTaskWidget::refreshWidgetsValues(UAVObject *obj)
{
    if (!m_isWidgetUpdatesAllowed) {
        return;
    }

    bool dirtyBack = isDirty();
    emit refreshWidgetsValuesRequested();
    foreach(WidgetBinding * binding, m_widgetBindingsPerObject.values(obj)) {
        if (binding->isEnabled() && binding->field() != NULL && binding->widget() != NULL) {
            setWidgetFromField(binding->widget(), binding->field(), binding->index(), binding->scale(), binding->isLimited());
        }
    }
    setDirty(dirtyBack);
}

void ConfigTaskWidget::updateObjectsFromWidgets()
{
    emit updateObjectsFromWidgetsRequested();

    foreach(WidgetBinding * binding, m_widgetBindingsPerObject) {
        if (binding->object() != NULL && binding->field() != NULL) {
            binding->updateObjectFieldFromValue();
        }
    }
}

void ConfigTaskWidget::helpButtonPressed()
{
    QString url = m_helpButtons.value((QPushButton *)sender(), QString());

    if (!url.isEmpty()) {
        QDesktopServices::openUrl(QUrl(url, QUrl::StrictMode));
    }
}

void ConfigTaskWidget::addApplySaveButtons(QPushButton *update, QPushButton *save)
{
    if (!m_saveButton) {
        m_saveButton = new SmartSaveButton(this);
        connect(m_saveButton, SIGNAL(preProcessOperations()), this, SLOT(updateObjectsFromWidgets()));
        connect(m_saveButton, SIGNAL(saveSuccessfull()), this, SLOT(clearDirty()));
        connect(m_saveButton, SIGNAL(beginOp()), this, SLOT(disableObjectUpdates()));
        connect(m_saveButton, SIGNAL(endOp()), this, SLOT(enableObjectUpdates()));
    }
    if (update && save) {
        m_saveButton->addButtons(save, update);
    } else if (update) {
        m_saveButton->addApplyButton(update);
    } else if (save) {
        m_saveButton->addSaveButton(save);
    }
    foreach(WidgetBinding * binding, m_widgetBindingsPerWidget) {
        m_saveButton->addObject((UAVDataObject *)binding->object());
    }
    updateEnableControls();
}

void ConfigTaskWidget::enableControls(bool enable)
{
    if (m_saveButton) {
        m_saveButton->enableControls(enable);
    }

    foreach(QPushButton * button, m_reloadButtons) {
        button->setEnabled(enable);
    }

    foreach(WidgetBinding * binding, m_widgetBindingsPerObject) {
        if (binding->isEnabled() && binding->widget()) {
            binding->widget()->setEnabled(enable);
            foreach(ShadowWidgetBinding * shadow, binding->shadows()) {
                shadow->widget()->setEnabled(enable);
            }
        }
    }
}

bool ConfigTaskWidget::shouldObjectBeSaved(UAVObject *object)
{
    Q_UNUSED(object);
    return true;
}

void ConfigTaskWidget::forceShadowUpdates()
{
    foreach(WidgetBinding * binding, m_widgetBindingsPerObject) {
        if (!binding->isEnabled()) {
            continue;
        }
        QVariant widgetValue = getVariantFromWidget(binding->widget(), binding->scale(), binding->units());

        foreach(ShadowWidgetBinding * shadow, binding->shadows()) {
            disconnectWidgetUpdatesToSlot(shadow->widget(), SLOT(widgetsContentsChanged()));

            checkWidgetsLimits(shadow->widget(), binding->field(), binding->index(), shadow->isLimited(), widgetValue, shadow->scale());
            setWidgetFromVariant(shadow->widget(), widgetValue, shadow->scale());

            emit widgetContentsChanged(shadow->widget());
            connectWidgetUpdatesToSlot(shadow->widget(), SLOT(widgetsContentsChanged()));
        }
    }
    setDirty(true);
}

void ConfigTaskWidget::widgetsContentsChanged()
{
    QWidget *emitter = ((QWidget *)sender());
    emit widgetContentsChanged(emitter);
    double scale;
    QVariant value;

    foreach(WidgetBinding * binding, m_widgetBindingsPerWidget.values(emitter)) {
        if (binding && binding->isEnabled()) {
            if (binding->widget() == emitter) {
                scale = binding->scale();
                checkWidgetsLimits(emitter, binding->field(), binding->index(), binding->isLimited(),
                                   getVariantFromWidget(emitter, scale, binding->units()), scale);
            } else {
                foreach(ShadowWidgetBinding * shadow, binding->shadows()) {
                    if (shadow->widget() == emitter) {
                        scale = shadow->scale();
                        checkWidgetsLimits(emitter, binding->field(), binding->index(), shadow->isLimited(),
                                           getVariantFromWidget(emitter, scale, binding->units()), scale);
                    }
                }
            }
            value = getVariantFromWidget(emitter, scale, binding->units());
            binding->setValue(value);

            if (binding->widget() != emitter) {
                disconnectWidgetUpdatesToSlot(binding->widget(), SLOT(widgetsContentsChanged()));

                checkWidgetsLimits(binding->widget(), binding->field(), binding->index(), binding->isLimited(),
                                   value, binding->scale());
                setWidgetFromVariant(binding->widget(), value, binding->scale());
                emit widgetContentsChanged(binding->widget());

                connectWidgetUpdatesToSlot(binding->widget(), SLOT(widgetsContentsChanged()));
            }
            foreach(ShadowWidgetBinding * shadow, binding->shadows()) {
                if (shadow->widget() != emitter) {
                    disconnectWidgetUpdatesToSlot(shadow->widget(), SLOT(widgetsContentsChanged()));

                    checkWidgetsLimits(shadow->widget(), binding->field(), binding->index(), shadow->isLimited(),
                                       value, shadow->scale());
                    setWidgetFromVariant(shadow->widget(), value, shadow->scale());
                    emit widgetContentsChanged(shadow->widget());

                    connectWidgetUpdatesToSlot(shadow->widget(), SLOT(widgetsContentsChanged()));
                }
            }
        }
    }
    if (m_saveButton) {
        m_saveButton->resetIcons();
    }
    setDirty(true);
}

void ConfigTaskWidget::clearDirty()
{
    setDirty(false);
}

void ConfigTaskWidget::setDirty(bool value)
{
    m_isDirty = value;
}

bool ConfigTaskWidget::isDirty()
{
    if (m_isConnected) {
        return m_isDirty;
    } else {
        return false;
    }
}

void ConfigTaskWidget::disableObjectUpdates()
{
    m_isWidgetUpdatesAllowed = false;
    foreach(WidgetBinding * binding, m_widgetBindingsPerWidget) {
        if (binding->object()) {
            disconnect(binding->object(), SIGNAL(objectUpdated(UAVObject *)), this, SLOT(refreshWidgetsValues(UAVObject *)));
        }
    }
}

void ConfigTaskWidget::enableObjectUpdates()
{
    m_isWidgetUpdatesAllowed = true;
    foreach(WidgetBinding * binding, m_widgetBindingsPerWidget) {
        if (binding->object()) {
            connect(binding->object(), SIGNAL(objectUpdated(UAVObject *)), this, SLOT(refreshWidgetsValues(UAVObject *)), Qt::UniqueConnection);
        }
    }
}

void ConfigTaskWidget::objectUpdated(UAVObject *object)
{
    m_updatedObjects[object] = true;
}

bool ConfigTaskWidget::allObjectsUpdated()
{
    bool result = true;

    foreach(UAVObject * object, m_updatedObjects.keys()) {
        result = result & m_updatedObjects[object];
    }
    return result;
}

void ConfigTaskWidget::addHelpButton(QPushButton *button, QString url)
{
    m_helpButtons.insert(button, url);
    connect(button, SIGNAL(clicked()), this, SLOT(helpButtonPressed()));
}

void ConfigTaskWidget::invalidateObjects()
{
    foreach(UAVObject * obj, m_updatedObjects.keys()) {
        m_updatedObjects[obj] = false;
    }
}

void ConfigTaskWidget::apply()
{
    if (m_saveButton) {
        m_saveButton->apply();
    }
}

void ConfigTaskWidget::save()
{
    if (m_saveButton) {
        m_saveButton->save();
    }
}

bool ConfigTaskWidget::addShadowWidgetBinding(QString objectName, QString fieldName, QWidget *widget, int index, double scale, bool isLimited,
                                              QList<int> *defaultReloadGroups, quint32 instID)
{
    foreach(WidgetBinding * binding, m_widgetBindingsPerWidget) {
        if (!binding->object() || !binding->widget() || !binding->field()) {
            continue;
        }
        if (binding->matches(objectName, fieldName, index, instID)) {
            binding->addShadow(widget, scale, isLimited);

            m_widgetBindingsPerWidget.insert(widget, binding);
            connectWidgetUpdatesToSlot(widget, SLOT(widgetsContentsChanged()));
            if (defaultReloadGroups) {
                addWidgetToReloadGroups(widget, defaultReloadGroups);
            }
            if (!binding->isEnabled()) {
                loadWidgetLimits(widget, binding->field(), binding->index(), isLimited, scale);
            }
            return true;
        }
    }
    return false;
}

void ConfigTaskWidget::autoLoadWidgets()
{
    QPushButton *saveButtonWidget  = NULL;
    QPushButton *applyButtonWidget = NULL;

    foreach(QWidget * widget, this->findChildren<QWidget *>()) {
        QVariant info = widget->property("objrelation");

        if (info.isValid()) {
            bindingStruct uiRelation;
            uiRelation.buttonType  = none;
            uiRelation.scale = 1;
            uiRelation.index = -1;
            uiRelation.elementName = QString();
            uiRelation.haslimits   = false;
            foreach(QString str, info.toStringList()) {
                QString prop  = str.split(":").at(0);
                QString value = str.split(":").at(1);

                if (prop == "objname") {
                    uiRelation.objectName = value;
                } else if (prop == "fieldname") {
                    uiRelation.fieldName = value;
                } else if (prop == "element") {
                    uiRelation.elementName = value;
                } else if (prop == "index") {
                    uiRelation.index = value.toInt();
                } else if (prop == "scale") {
                    if (value == "null") {
                        uiRelation.scale = 1;
                    } else {
                        uiRelation.scale = value.toDouble();
                    }
                } else if (prop == "haslimits") {
                    if (value == "yes") {
                        uiRelation.haslimits = true;
                    } else {
                        uiRelation.haslimits = false;
                    }
                } else if (prop == "button") {
                    if (value == "save") {
                        uiRelation.buttonType = save_button;
                    } else if (value == "apply") {
                        uiRelation.buttonType = apply_button;
                    } else if (value == "reload") {
                        uiRelation.buttonType = reload_button;
                    } else if (value == "default") {
                        uiRelation.buttonType = default_button;
                    } else if (value == "help") {
                        uiRelation.buttonType = help_button;
                    }
                } else if (prop == "buttongroup") {
                    foreach(QString s, value.split(",")) {
                        uiRelation.buttonGroup.append(s.toInt());
                    }
                } else if (prop == "url") {
                    uiRelation.url = str.mid(str.indexOf(":") + 1);
                }
            }
            if (!uiRelation.buttonType == none) {
                QPushButton *button = NULL;
                switch (uiRelation.buttonType) {
                case save_button:
                    saveButtonWidget = qobject_cast<QPushButton *>(widget);
                    if (saveButtonWidget) {
                        addApplySaveButtons(NULL, saveButtonWidget);
                    }
                    break;
                case apply_button:
                    applyButtonWidget = qobject_cast<QPushButton *>(widget);
                    if (applyButtonWidget) {
                        addApplySaveButtons(applyButtonWidget, NULL);
                    }
                    break;
                case default_button:
                    button = qobject_cast<QPushButton *>(widget);
                    if (button) {
                        addDefaultButton(button, uiRelation.buttonGroup.at(0));
                    }
                    break;
                case reload_button:
                    button = qobject_cast<QPushButton *>(widget);
                    if (button) {
                        addReloadButton(button, uiRelation.buttonGroup.at(0));
                    }
                    break;
                case help_button:
                    button = qobject_cast<QPushButton *>(widget);
                    if (button) {
                        addHelpButton(button, uiRelation.url);
                    }
                    break;

                default:
                    break;
                }
            } else {
                QWidget *wid = qobject_cast<QWidget *>(widget);
                if (wid) {
                    if (uiRelation.index != -1) {
                        addWidgetBinding(uiRelation.objectName, uiRelation.fieldName, wid, uiRelation.index, uiRelation.scale, uiRelation.haslimits, &uiRelation.buttonGroup);
                    } else {
                        addWidgetBinding(uiRelation.objectName, uiRelation.fieldName, wid, uiRelation.elementName, uiRelation.scale, uiRelation.haslimits, &uiRelation.buttonGroup);
                    }
                }
            }
        }
    }
    refreshWidgetsValues();
    forceShadowUpdates();

    /*
       foreach(WidgetBinding * binding, m_widgetBindingsPerObject) {
        if (binding->widget()) {
            qDebug() << "Binding  :" << binding->widget()->objectName();
            qDebug() << "  Object :" << binding->object()->getName();
            qDebug() << "  Field  :" << binding->field()->getName();
            qDebug() << "  Scale  :" << binding->scale();
            qDebug() << "  Enabled:" << binding->isEnabled();
        }
        foreach(ShadowWidgetBinding * shadow, binding->shadows()) {
            if (shadow->widget()) {
                qDebug() << "  Shadow:" << shadow->widget()->objectName();
                qDebug() << "  Scale :" << shadow->scale();
            }
        }
       }
     */
}

void ConfigTaskWidget::addWidgetToReloadGroups(QWidget *widget, QList<int> *reloadGroupIDs)
{
    foreach(WidgetBinding * binding, m_widgetBindingsPerWidget) {
        bool addBinding = false;

        if (binding->widget() == widget) {
            addBinding = true;
        } else {
            foreach(ShadowWidgetBinding * shadow, binding->shadows()) {
                if (shadow->widget() == widget) {
                    addBinding = true;
                }
            }
        }
        if (addBinding) {
            foreach(int groupID, *reloadGroupIDs) {
                m_reloadGroups.insert(groupID, binding);
            }
        }
    }
}

void ConfigTaskWidget::addDefaultButton(QPushButton *button, int buttonGroup)
{
    button->setProperty("group", buttonGroup);
    connect(button, SIGNAL(clicked()), this, SLOT(defaultButtonClicked()));
}

void ConfigTaskWidget::addReloadButton(QPushButton *button, int buttonGroup)
{
    button->setProperty("group", buttonGroup);
    m_reloadButtons.append(button);
    connect(button, SIGNAL(clicked()), this, SLOT(reloadButtonClicked()));
}

void ConfigTaskWidget::defaultButtonClicked()
{
    int groupID = sender()->property("group").toInt();
    emit defaultRequested(groupID);

    QList<WidgetBinding *> bindings = m_reloadGroups.values(groupID);
    foreach(WidgetBinding * binding, bindings) {
        if (!binding->isEnabled() || !binding->object() || !binding->field()) {
            continue;
        }
        UAVDataObject *temp = ((UAVDataObject *)binding->object())->dirtyClone();
        setWidgetFromField(binding->widget(), temp->getField(binding->field()->getName()), binding->index(), binding->scale(), binding->isLimited());
    }
}

void ConfigTaskWidget::reloadButtonClicked()
{
    if (m_realtimeUpdateTimer) {
        return;
    }
    int groupID = sender()->property("group").toInt();
    QList<WidgetBinding *> bindings = m_reloadGroups.values(groupID);
    if (!bindings.isEmpty()) {
        return;
    }
    ObjectPersistence *objper = dynamic_cast<ObjectPersistence *>(getObjectManager()->getObject(ObjectPersistence::NAME));
    m_realtimeUpdateTimer = new QTimer(this);
    QEventLoop *eventLoop     = new QEventLoop(this);
    connect(m_realtimeUpdateTimer, SIGNAL(timeout()), eventLoop, SLOT(quit()));
    connect(objper, SIGNAL(objectUpdated(UAVObject *)), eventLoop, SLOT(quit()));

    QList<objectComparator> temp;
    foreach(WidgetBinding * binding, bindings) {
        if (binding->isEnabled() && binding->object() != NULL) {
            objectComparator value;
            value.objid     = binding->object()->getObjID();
            value.objinstid = binding->object()->getInstID();
            if (temp.contains(value)) {
                continue;
            } else {
                temp.append(value);
            }
            ObjectPersistence::DataFields data;
            data.Operation  = ObjectPersistence::OPERATION_LOAD;
            data.Selection  = ObjectPersistence::SELECTION_SINGLEOBJECT;
            data.ObjectID   = binding->object()->getObjID();
            data.InstanceID = binding->object()->getInstID();
            objper->setData(data);
            objper->updated();
            m_realtimeUpdateTimer->start(500);
            eventLoop->exec();
            if (m_realtimeUpdateTimer->isActive()) {
                binding->object()->requestUpdate();
                if (binding->widget()) {
                    setWidgetFromField(binding->widget(), binding->field(), binding->index(), binding->scale(), binding->isLimited());
                }
            }
            m_realtimeUpdateTimer->stop();
        }
    }
    if (eventLoop) {
        delete eventLoop;
        eventLoop = NULL;
    }
    if (m_realtimeUpdateTimer) {
        delete m_realtimeUpdateTimer;
        m_realtimeUpdateTimer = NULL;
    }
}

void ConfigTaskWidget::connectWidgetUpdatesToSlot(QWidget *widget, const char *function)
{
    if (!widget) {
        return;
    }
    if (QComboBox * cb = qobject_cast<QComboBox *>(widget)) {
        connect(cb, SIGNAL(currentIndexChanged(int)), this, function, Qt::UniqueConnection);
    } else if (QSlider * cb = qobject_cast<QSlider *>(widget)) {
        connect(cb, SIGNAL(valueChanged(int)), this, function, Qt::UniqueConnection);
    } else if (MixerCurveWidget * cb = qobject_cast<MixerCurveWidget *>(widget)) {
        connect(cb, SIGNAL(curveUpdated()), this, function, Qt::UniqueConnection);
    } else if (QTableWidget * cb = qobject_cast<QTableWidget *>(widget)) {
        connect(cb, SIGNAL(cellChanged(int, int)), this, function, Qt::UniqueConnection);
    } else if (QSpinBox * cb = qobject_cast<QSpinBox *>(widget)) {
        connect(cb, SIGNAL(valueChanged(int)), this, function, Qt::UniqueConnection);
    } else if (QDoubleSpinBox * cb = qobject_cast<QDoubleSpinBox *>(widget)) {
        connect(cb, SIGNAL(valueChanged(double)), this, function, Qt::UniqueConnection);
    } else if (QCheckBox * cb = qobject_cast<QCheckBox *>(widget)) {
        connect(cb, SIGNAL(stateChanged(int)), this, function, Qt::UniqueConnection);
    } else if (QPushButton * cb = qobject_cast<QPushButton *>(widget)) {
        connect(cb, SIGNAL(clicked()), this, function, Qt::UniqueConnection);
    } else {
        qDebug() << __FUNCTION__ << "widget binding not implemented" << widget->metaObject()->className();
    }
}

void ConfigTaskWidget::disconnectWidgetUpdatesToSlot(QWidget *widget, const char *function)
{
    if (!widget) {
        return;
    }
    if (QComboBox * cb = qobject_cast<QComboBox *>(widget)) {
        disconnect(cb, SIGNAL(currentIndexChanged(int)), this, function);
    } else if (QSlider * cb = qobject_cast<QSlider *>(widget)) {
        disconnect(cb, SIGNAL(valueChanged(int)), this, function);
    } else if (MixerCurveWidget * cb = qobject_cast<MixerCurveWidget *>(widget)) {
        disconnect(cb, SIGNAL(curveUpdated()), this, function);
    } else if (QTableWidget * cb = qobject_cast<QTableWidget *>(widget)) {
        disconnect(cb, SIGNAL(cellChanged(int, int)), this, function);
    } else if (QSpinBox * cb = qobject_cast<QSpinBox *>(widget)) {
        disconnect(cb, SIGNAL(valueChanged(int)), this, function);
    } else if (QDoubleSpinBox * cb = qobject_cast<QDoubleSpinBox *>(widget)) {
        disconnect(cb, SIGNAL(valueChanged(double)), this, function);
    } else if (QCheckBox * cb = qobject_cast<QCheckBox *>(widget)) {
        disconnect(cb, SIGNAL(stateChanged(int)), this, function);
    } else if (QPushButton * cb = qobject_cast<QPushButton *>(widget)) {
        disconnect(cb, SIGNAL(clicked()), this, function);
    } else {
        qDebug() << __FUNCTION__ << "widget binding not implemented" << widget->metaObject()->className();
    }
}

QVariant ConfigTaskWidget::getVariantFromWidget(QWidget *widget, double scale, QString units)
{
    if (QComboBox * cb = qobject_cast<QComboBox *>(widget)) {
        return (QString)cb->currentText();
    } else if (QDoubleSpinBox * cb = qobject_cast<QDoubleSpinBox *>(widget)) {
        return (double)(cb->value() * scale);
    } else if (QSpinBox * cb = qobject_cast<QSpinBox *>(widget)) {
        return (double)(cb->value() * scale);
    } else if (QSlider * cb = qobject_cast<QSlider *>(widget)) {
        return (double)(cb->value() * scale);
    } else if (QCheckBox * cb = qobject_cast<QCheckBox *>(widget)) {
        return (QString)(cb->isChecked() ? "TRUE" : "FALSE");
    } else if (QLineEdit * cb = qobject_cast<QLineEdit *>(widget)) {
        QString value = (QString)cb->displayText();
        if (units == "hex") {
            bool ok;
            return value.toUInt(&ok, 16);
        } else {
            return value;
        }
    } else {
        return QVariant();
    }
}

bool ConfigTaskWidget::setWidgetFromVariant(QWidget *widget, QVariant value, double scale, QString units)
{
    if (QComboBox * cb = qobject_cast<QComboBox *>(widget)) {
        cb->setCurrentIndex(cb->findText(value.toString()));
        return true;
    } else if (QLabel * cb = qobject_cast<QLabel *>(widget)) {
        if (scale == 0) {
            cb->setText(value.toString());
        } else {
            cb->setText(QString::number((value.toDouble() / scale)));
        }
        return true;
    } else if (QDoubleSpinBox * cb = qobject_cast<QDoubleSpinBox *>(widget)) {
        cb->setValue((double)(value.toDouble() / scale));
        return true;
    } else if (QSpinBox * cb = qobject_cast<QSpinBox *>(widget)) {
        cb->setValue((int)qRound(value.toDouble() / scale));
        return true;
    } else if (QSlider * cb = qobject_cast<QSlider *>(widget)) {
        cb->setValue((int)qRound(value.toDouble() / scale));
        return true;
    } else if (QCheckBox * cb = qobject_cast<QCheckBox *>(widget)) {
        bool bvalue = value.toString() == "TRUE";
        cb->setChecked(bvalue);
        return true;
    } else if (QLineEdit * cb = qobject_cast<QLineEdit *>(widget)) {
        if ((scale == 0) || (scale == 1)) {
            if (units == "hex") {
                cb->setText(QString::number(value.toUInt(), 16).toUpper());
            } else {
                cb->setText(value.toString());
            }
        } else {
            cb->setText(QString::number((value.toDouble() / scale)));
        }
        return true;
    } else {
        return false;
    }
}

bool ConfigTaskWidget::setWidgetFromVariant(QWidget *widget, QVariant value, double scale)
{
    return setWidgetFromVariant(widget, value, scale, QString(""));
}

bool ConfigTaskWidget::setWidgetFromField(QWidget *widget, UAVObjectField *field, int index, double scale, bool hasLimits)
{
    if (!widget || !field) {
        return false;
    }
    if (QComboBox * cb = qobject_cast<QComboBox *>(widget)) {
        if (cb->count() == 0) {
            loadWidgetLimits(cb, field, index, hasLimits, scale);
        }
    }
    QVariant value = field->getValue(index);
    checkWidgetsLimits(widget, field, index, hasLimits, value, scale);
    bool result    = setWidgetFromVariant(widget, value, scale, field->getUnits());
    if (result) {
        return true;
    } else {
        qDebug() << __FUNCTION__ << "widget to uavobject relation not implemented" << widget->metaObject()->className();
        return false;
    }
}

void ConfigTaskWidget::checkWidgetsLimits(QWidget *widget, UAVObjectField *field, int index, bool hasLimits, QVariant value, double scale)
{
    if (!hasLimits) {
        return;
    }
    if (!field->isWithinLimits(value, index, m_currentBoardId)) {
        if (!widget->property("styleBackup").isValid()) {
            widget->setProperty("styleBackup", widget->styleSheet());
        }
        widget->setStyleSheet(m_outOfLimitsStyle);
        widget->setProperty("wasOverLimits", (bool)true);
        if (QComboBox * cb = qobject_cast<QComboBox *>(widget)) {
            if (cb->findText(value.toString()) == -1) {
                cb->addItem(value.toString());
            }
        } else if (QDoubleSpinBox * cb = qobject_cast<QDoubleSpinBox *>(widget)) {
            if ((double)(value.toDouble() / scale) > cb->maximum()) {
                cb->setMaximum((double)(value.toDouble() / scale));
            } else if ((double)(value.toDouble() / scale) < cb->minimum()) {
                cb->setMinimum((double)(value.toDouble() / scale));
            }
        } else if (QSpinBox * cb = qobject_cast<QSpinBox *>(widget)) {
            if ((int)qRound(value.toDouble() / scale) > cb->maximum()) {
                cb->setMaximum((int)qRound(value.toDouble() / scale));
            } else if ((int)qRound(value.toDouble() / scale) < cb->minimum()) {
                cb->setMinimum((int)qRound(value.toDouble() / scale));
            }
        } else if (QSlider * cb = qobject_cast<QSlider *>(widget)) {
            if ((int)qRound(value.toDouble() / scale) > cb->maximum()) {
                cb->setMaximum((int)qRound(value.toDouble() / scale));
            } else if ((int)qRound(value.toDouble() / scale) < cb->minimum()) {
                cb->setMinimum((int)qRound(value.toDouble() / scale));
            }
        }
    } else if (widget->property("wasOverLimits").isValid()) {
        if (widget->property("wasOverLimits").toBool()) {
            widget->setProperty("wasOverLimits", (bool)false);
            if (widget->property("styleBackup").isValid()) {
                QString style = widget->property("styleBackup").toString();
                widget->setStyleSheet(style);
            }
            loadWidgetLimits(widget, field, index, hasLimits, scale);
        }
    }
}

void ConfigTaskWidget::loadWidgetLimits(QWidget *widget, UAVObjectField *field, int index, bool hasLimits, double scale)
{
    if (!widget || !field) {
        return;
    }
    if (QComboBox * cb = qobject_cast<QComboBox *>(widget)) {
        cb->clear();
        QStringList option = field->getOptions();
        if (hasLimits) {
            foreach(QString str, option) {
                if (field->isWithinLimits(str, index, m_currentBoardId)) {
                    cb->addItem(str);
                }
            }
        } else {
            cb->addItems(option);
        }
    }
    if (!hasLimits) {
        return;
    } else if (QDoubleSpinBox * cb = qobject_cast<QDoubleSpinBox *>(widget)) {
        if (field->getMaxLimit(index).isValid()) {
            cb->setMaximum((double)(field->getMaxLimit(index, m_currentBoardId).toDouble() / scale));
        }
        if (field->getMinLimit(index, m_currentBoardId).isValid()) {
            cb->setMinimum((double)(field->getMinLimit(index, m_currentBoardId).toDouble() / scale));
        }
    } else if (QSpinBox * cb = qobject_cast<QSpinBox *>(widget)) {
        if (field->getMaxLimit(index, m_currentBoardId).isValid()) {
            cb->setMaximum((int)qRound(field->getMaxLimit(index, m_currentBoardId).toDouble() / scale));
        }
        if (field->getMinLimit(index, m_currentBoardId).isValid()) {
            cb->setMinimum((int)qRound(field->getMinLimit(index, m_currentBoardId).toDouble() / scale));
        }
    } else if (QSlider * cb = qobject_cast<QSlider *>(widget)) {
        if (field->getMaxLimit(index, m_currentBoardId).isValid()) {
            cb->setMaximum((int)qRound(field->getMaxLimit(index, m_currentBoardId).toDouble() / scale));
        }
        if (field->getMinLimit(index, m_currentBoardId).isValid()) {
            cb->setMinimum((int)(field->getMinLimit(index, m_currentBoardId).toDouble() / scale));
        }
    }
}

UAVObject *ConfigTaskWidget::getObject(const QString name, quint32 instId)
{
    return m_pluginManager->getObject<UAVObjectManager>()->getObject(name, instId);
}

QString ConfigTaskWidget::mapObjectName(const QString objectName)
{
    return objectName;
}

void ConfigTaskWidget::updateEnableControls()
{
    TelemetryManager *telMngr = m_pluginManager->getObject<TelemetryManager>();

    Q_ASSERT(telMngr);
    enableControls(telMngr->isConnected());
}

void ConfigTaskWidget::disableMouseWheelEvents()
{
    // Disable mouse wheel events
    foreach(QSpinBox * sp, findChildren<QSpinBox *>()) {
        sp->installEventFilter(this);
    }
    foreach(QDoubleSpinBox * sp, findChildren<QDoubleSpinBox *>()) {
        sp->installEventFilter(this);
    }
    foreach(QSlider * sp, findChildren<QSlider *>()) {
        sp->installEventFilter(this);
    }
    foreach(QComboBox * sp, findChildren<QComboBox *>()) {
        sp->installEventFilter(this);
    }
}

bool ConfigTaskWidget::eventFilter(QObject *obj, QEvent *evt)
{
    // Filter all wheel events, and ignore them
    if (evt->type() == QEvent::Wheel &&
        (qobject_cast<QAbstractSpinBox *>(obj) ||
         qobject_cast<QComboBox *>(obj) ||
         qobject_cast<QAbstractSlider *>(obj))) {
        evt->ignore();
        return true;
    }
    return QWidget::eventFilter(obj, evt);
}

WidgetBinding::WidgetBinding(QWidget *widget, UAVObject *object, UAVObjectField *field, int index, double scale, bool isLimited) :
    ShadowWidgetBinding(widget, scale, isLimited), m_isEnabled(true)
{
    m_object = object;
    m_field  = field;
    m_index  = index;
}

WidgetBinding::~WidgetBinding()
{}

QString WidgetBinding::units() const
{
    if (m_field) {
        return m_field->getUnits();
    }
    return QString("");
}

UAVObject *WidgetBinding::object() const
{
    return m_object;
}

UAVObjectField *WidgetBinding::field() const
{
    return m_field;
}

int WidgetBinding::index() const
{
    return m_index;
}

QList<ShadowWidgetBinding *> WidgetBinding::shadows() const
{
    return m_shadows;
}

void WidgetBinding::addShadow(QWidget *widget, double scale, bool isLimited)
{
    ShadowWidgetBinding *shadow = NULL;

    // Prefer anything else to QLabel and prefer QDoubleSpinBox to anything else
    if ((qobject_cast<QLabel *>(m_widget) && !qobject_cast<QLabel *>(widget)) ||
        (!qobject_cast<QDoubleSpinBox *>(m_widget) && qobject_cast<QDoubleSpinBox *>(widget))) {
        shadow      = new ShadowWidgetBinding(m_widget, m_scale, m_isLimited);
        m_isLimited = isLimited;
        m_scale     = scale;
        m_widget    = widget;
    } else {
        shadow = new ShadowWidgetBinding(widget, scale, isLimited);
    }
    m_shadows.append(shadow);
}

bool WidgetBinding::matches(QString objectName, QString fieldName, int index, quint32 instanceId)
{
    if (m_object && m_field) {
        return m_object->getName() == objectName && m_object->getInstID() == instanceId &&
               m_field->getName() == fieldName && m_index == index;
    } else {
        return false;
    }
}

bool WidgetBinding::isEnabled() const
{
    return m_isEnabled;
}

void WidgetBinding::setIsEnabled(bool isEnabled)
{
    m_isEnabled = isEnabled;
}

QVariant WidgetBinding::value() const
{
    return m_value;
}

void WidgetBinding::setValue(const QVariant &value)
{
    m_value = value;
    /*
       if (m_object && m_field) {
        qDebug() << "WidgetBinding" << m_object->getName() << ":" << m_field->getName() << "value =" << value.toString();
       }
     */
}

void WidgetBinding::updateObjectFieldFromValue()
{
    if (m_value.isValid()) {
        m_field->setValue(m_value, m_index);
    }
}

ShadowWidgetBinding::ShadowWidgetBinding(QWidget *widget, double scale, bool isLimited)
{
    m_widget    = widget;
    m_scale     = scale;
    m_isLimited = isLimited;
}

ShadowWidgetBinding::~ShadowWidgetBinding()
{}

QWidget *ShadowWidgetBinding::widget() const
{
    return m_widget;
}

double ShadowWidgetBinding::scale() const
{
    return m_scale;
}

bool ShadowWidgetBinding::isLimited() const
{
    return m_isLimited;
}
