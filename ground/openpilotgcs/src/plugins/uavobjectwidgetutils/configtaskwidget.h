/**
 ******************************************************************************
 *
 * @file       configtaskwidget.h
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
#ifndef CONFIGTASKWIDGET_H
#define CONFIGTASKWIDGET_H

#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include "uavobjectutilmanager.h"
#include <QQueue>
#include <QWidget>
#include <QList>
#include <QLabel>
#include "smartsavebutton.h"
#include "mixercurvewidget.h"
#include <QTableWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include "uavobjectwidgetutils_global.h"
#include <QDesktopServices>
#include <QUrl>
#include <QEvent>

class ShadowWidgetBinding : public QObject {
    Q_OBJECT
public:
    ShadowWidgetBinding(QWidget *widget, double scale, bool isLimited);
    ~ShadowWidgetBinding();
    QWidget *widget() const;
    double scale() const;
    bool isLimited() const;

protected:
    QWidget *m_widget;
    double m_scale;
    bool m_isLimited;
};

class WidgetBinding : public ShadowWidgetBinding {
    Q_OBJECT
public:
    WidgetBinding(QWidget *widget, UAVObject *object, UAVObjectField *field, int index, double scale, bool isLimited);
    ~WidgetBinding();

    QString units() const;
    UAVObject *object() const;
    UAVObjectField *field() const;
    int index() const;
    QList<ShadowWidgetBinding *> shadows() const;

    void addShadow(QWidget *widget, double scale, bool isLimited);
    bool matches(QString objectName, QString fieldName, int index, quint32 instanceId);

    bool isEnabled() const;
    void setIsEnabled(bool isEnabled);

    QVariant value() const;
    void setValue(const QVariant &value);

    void updateObjectFieldFromValue();

private:
    UAVObject *m_object;
    UAVObjectField *m_field;
    int m_index;
    bool m_isEnabled;
    QList<ShadowWidgetBinding *> m_shadows;
    QVariant m_value;
};

class UAVOBJECTWIDGETUTILS_EXPORT ConfigTaskWidget : public QWidget {
    Q_OBJECT

public:
    ConfigTaskWidget(QWidget *parent = 0);
    virtual ~ConfigTaskWidget();

    void disableMouseWheelEvents();
    bool eventFilter(QObject *obj, QEvent *evt);

    void saveObjectToSD(UAVObject *obj);
    UAVObjectManager *getObjectManager();
    static double listMean(QList<double> list);
    static double listVar(QList<double> list);

    void addUAVObject(QString objectName, QList<int> *reloadGroups = NULL);
    void addUAVObject(UAVObject *objectName, QList<int> *reloadGroups = NULL);

    void addWidget(QWidget *widget);

    void addWidgetBinding(QString objectName, QString fieldName, QWidget *widget, int index = 0, double scale = 1,
                          bool isLimited = false, QList<int> *reloadGroupIDs = 0, quint32 instID = 0);
    void addWidgetBinding(UAVObject *object, UAVObjectField *field, QWidget *widget, int index = 0, double scale = 1,
                          bool isLimited = false, QList<int> *reloadGroupIDs = 0, quint32 instID = 0);

    void addWidgetBinding(QString objectName, QString fieldName, QWidget *widget, QString elementName, double scale,
                          bool isLimited = false, QList<int> *reloadGroupIDs = 0, quint32 instID = 0);
    void addWidgetBinding(UAVObject *object, UAVObjectField *field, QWidget *widget, QString elementName, double scale,
                          bool isLimited = false, QList<int> *reloadGroupIDs = 0, quint32 instID = 0);

    void addWidgetBinding(QString objectName, QString fieldName, QWidget *widget, QString elementName);
    void addWidgetBinding(UAVObject *object, UAVObjectField *field, QWidget *widget, QString elementName);

    void addApplySaveButtons(QPushButton *update, QPushButton *save);
    void addReloadButton(QPushButton *button, int buttonGroup);
    void addDefaultButton(QPushButton *button, int buttonGroup);

    void addWidgetToReloadGroups(QWidget *widget, QList<int> *reloadGroupIDs);

    bool addShadowWidgetBinding(QString objectName, QString fieldName, QWidget *widget, int index = 0, double scale = 1,
                                bool isLimited = false, QList<int> *m_reloadGroups = NULL, quint32 instID = 0);

    void autoLoadWidgets();

    bool isDirty();
    void setDirty(bool value);

    bool allObjectsUpdated();
    void setOutOfLimitsStyle(QString style)
    {
        m_outOfLimitsStyle = style;
    }
    void addHelpButton(QPushButton *button, QString url);
    void forceShadowUpdates();
    void forceConnectedState();
    virtual bool shouldObjectBeSaved(UAVObject *object);

public slots:
    void onAutopilotDisconnect();
    void onAutopilotConnect();
    void invalidateObjects();
    void apply();
    void save();
    void setWidgetBindingObjectEnabled(QString objectName, bool enabled);

signals:
    // fired when a widgets contents changes
    void widgetContentsChanged(QWidget *widget);
    // fired when the framework requests that the widgets values be populated, use for custom behaviour
    void populateWidgetsRequested();
    // fired when the framework requests that the widgets values be refreshed, use for custom behaviour
    void refreshWidgetsValuesRequested();
    // fired when the framework requests that the UAVObject values be updated from the widgets value, use for custom behaviour
    void updateObjectsFromWidgetsRequested();
    // fired when the autopilot connects
    void autoPilotConnected();
    // fired when the autopilot disconnects
    void autoPilotDisconnected();
    void defaultRequested(int group);

private slots:
    void objectUpdated(UAVObject *object);
    void defaultButtonClicked();
    void reloadButtonClicked();

private:
    struct objectComparator {
        quint32 objid;
        quint32 objinstid;
        bool operator==(const objectComparator & lhs)
        {
            return lhs.objid == this->objid && lhs.objinstid == this->objinstid;
        }
    };

    enum buttonTypeEnum { none, save_button, apply_button, reload_button, default_button, help_button };
    struct bindingStruct {
        QString objname;
        QString fieldname;
        QString element;
        QString url;
        buttonTypeEnum buttonType;
        QList<int>     buttonGroup;
        double  scale;
        bool    haslimits;
    };

    int m_currentBoardId;
    bool m_isConnected;
    bool m_isWidgetUpdatesAllowed;
    QStringList m_objects;

    QMultiHash<int, WidgetBinding *> m_reloadGroups;
    QMultiHash<QWidget *, WidgetBinding *> m_widgetBindingsPerWidget;
    QMultiHash<UAVObject *, WidgetBinding *> m_widgetBindingsPerObject;

    ExtensionSystem::PluginManager *m_pluginManager;
    UAVObjectUtilManager *m_objectUtilManager;
    SmartSaveButton *m_saveButton;
    QHash<UAVObject *, bool> m_updatedObjects;
    QHash<QPushButton *, QString> m_helpButtons;
    QList<QPushButton *> m_reloadButtons;
    bool m_isDirty;
    QString m_outOfLimitsStyle;
    QTimer *m_realtimeUpdateTimer;

    bool setWidgetFromField(QWidget *widget, UAVObjectField *field, int index, double scale, bool hasLimits);

    QVariant getVariantFromWidget(QWidget *widget, double scale, const QString units);
    bool setWidgetFromVariant(QWidget *widget, QVariant value, double scale, QString units);
    bool setWidgetFromVariant(QWidget *widget, QVariant value, double scale);

    void connectWidgetUpdatesToSlot(QWidget *widget, const char *function);
    void disconnectWidgetUpdatesToSlot(QWidget *widget, const char *function);

    void loadWidgetLimits(QWidget *widget, UAVObjectField *field, int index, bool hasLimits, double sclale);

    int fieldIndexFromElementName(QString objectName, QString fieldName, QString elementName);

    void doAddWidgetBinding(QString objectName, QString fieldName, QWidget *widget, int index = 0, double scale = 1,
                            bool isLimited = false, QList<int> *reloadGroupIDs = 0, quint32 instID = 0);

protected slots:
    virtual void disableObjectUpdates();
    virtual void enableObjectUpdates();
    virtual void clearDirty();
    virtual void widgetsContentsChanged();
    virtual void populateWidgets();
    virtual void refreshWidgetsValues(UAVObject *obj = NULL);
    virtual void updateObjectsFromWidgets();
    virtual void helpButtonPressed();

protected:
    virtual void enableControls(bool enable);
    virtual QString mapObjectName(const QString objectName);
    virtual UAVObject *getObject(const QString name, quint32 instId = 0);
    void checkWidgetsLimits(QWidget *widget, UAVObjectField *field, int index, bool hasLimits, QVariant value, double scale);
    void updateEnableControls();
};

#endif // CONFIGTASKWIDGET_H
