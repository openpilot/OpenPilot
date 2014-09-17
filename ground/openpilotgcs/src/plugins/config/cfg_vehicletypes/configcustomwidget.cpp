/**
 ******************************************************************************
 *
 * @file       configcustomwidget.cpp
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief custom configuration panel
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
#include "configcustomwidget.h"
#include "mixersettings.h"

#include <QDebug>
#include <QStringList>
#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QBrush>
#include <math.h>
#include <QMessageBox>

QStringList ConfigCustomWidget::getChannelDescriptions()
{
    QStringList channelDesc;

    for (int i = 0; i < (int)VehicleConfig::CHANNEL_NUMELEM; i++) {
        channelDesc.append(QString("-"));
    }
    // get the gui config data
    GUIConfigDataUnion configData  = getConfigData();
    customGUISettingsStruct custom = configData.custom;

    if (custom.Motor1 > 0 && custom.Motor1 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.Motor1 - 1] = QString("Motor1");
    }
    if (custom.Motor2 > 0 && custom.Motor2 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.Motor2 - 1] = QString("Motor2");
    }
    if (custom.Motor3 > 0 && custom.Motor3 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.Motor3 - 1] = QString("Motor3");
    }
    if (custom.Motor4 > 0 && custom.Motor4 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.Motor4 - 1] = QString("Motor4");
    }
    if (custom.Motor5 > 0 && custom.Motor5 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.Motor5 - 1] = QString("Motor5");
    }
    if (custom.Motor6 > 0 && custom.Motor6 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.Motor6 - 1] = QString("Motor6");
    }
    if (custom.Motor7 > 0 && custom.Motor7 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.Motor7 - 1] = QString("Motor7");
    }
    if (custom.Motor8 > 0 && custom.Motor8 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.Motor8 - 1] = QString("Motor8");
    }

    if (custom.RevMotor1 > 0 && custom.RevMotor1 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.RevMotor1 - 1] = QString("RevMotor1");
    }
    if (custom.RevMotor2 > 0 && custom.RevMotor2 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.RevMotor2 - 1] = QString("RevMotor2");
    }
    if (custom.RevMotor3 > 0 && custom.RevMotor3 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.RevMotor3 - 1] = QString("RevMotor3");
    }
    if (custom.RevMotor4 > 0 && custom.RevMotor4 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.RevMotor4 - 1] = QString("RevMotor4");
    }
    if (custom.RevMotor5 > 0 && custom.RevMotor5 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.RevMotor5 - 1] = QString("RevMotor5");
    }
    if (custom.RevMotor6 > 0 && custom.RevMotor6 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.RevMotor6 - 1] = QString("RevMotor6");
    }
    if (custom.RevMotor7 > 0 && custom.RevMotor7 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.RevMotor7 - 1] = QString("RevMotor7");
    }
    if (custom.RevMotor8 > 0 && custom.RevMotor8 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.RevMotor8 - 1] = QString("RevMotor8");
    }

    if (custom.Servo1 > 0 && custom.Servo1 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.Servo1 - 1] = QString("Servo1");
    }
    if (custom.Servo2 > 0 && custom.Servo2 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.Servo2 - 1] = QString("Servo2");
    }
    if (custom.Servo3 > 0 && custom.Servo3 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.Servo3 - 1] = QString("Servo3");
    }
    if (custom.Servo4 > 0 && custom.Servo4 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.Servo4 - 1] = QString("Servo4");
    }
    if (custom.Servo5 > 0 && custom.Servo5 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.Servo5 - 1] = QString("Servo5");
    }
    if (custom.Servo6 > 0 && custom.Servo6 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.Servo6 - 1] = QString("Servo6");
    }
    if (custom.Servo7 > 0 && custom.Servo7 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.Servo7 - 1] = QString("Servo7");
    }
    if (custom.Servo8 > 0 && custom.Servo8 <= VehicleConfig::CHANNEL_NUMELEM) {
        channelDesc[custom.Servo8 - 1] = QString("Servo8");
    }
    return channelDesc;
}

ConfigCustomWidget::ConfigCustomWidget(QWidget *parent) :
    VehicleConfig(parent), m_aircraft(new Ui_CustomConfigWidget())
{
    m_aircraft->setupUi(this);
    m_aircraft->customMixerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Put combo boxes in line one of the custom mixer table:
    UAVDataObject *mixer  = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);

    UAVObjectField *field = mixer->getField(QString("Mixer1Type"));
    QStringList list = field->getOptions();
    for (int i = 0; i < (int)VehicleConfig::CHANNEL_NUMELEM; i++) {
        QComboBox *qb = new QComboBox(m_aircraft->customMixerTable);
        qb->addItems(list);
        m_aircraft->customMixerTable->setCellWidget(0, i, qb);
    }

    SpinBoxDelegate *sbd = new SpinBoxDelegate();
    for (int i = 1; i < (int)VehicleConfig::CHANNEL_NUMELEM; i++) {
        m_aircraft->customMixerTable->setItemDelegateForRow(i, sbd);
    }
}

ConfigCustomWidget::~ConfigCustomWidget()
{
    delete m_aircraft;
}

void ConfigCustomWidget::setupUI(QString frameType)
{
    Q_UNUSED(frameType);
    Q_ASSERT(m_aircraft);
}

void ConfigCustomWidget::registerWidgets(ConfigTaskWidget &parent)
{
    parent.addWidget(m_aircraft->customMixerTable);
    parent.addWidget(m_aircraft->customThrottle1Curve->getCurveWidget());
    parent.addWidget(m_aircraft->customThrottle1Curve);
    parent.addWidget(m_aircraft->customThrottle2Curve->getCurveWidget());
    parent.addWidget(m_aircraft->customThrottle2Curve);
}

void ConfigCustomWidget::resetActuators(GUIConfigDataUnion *configData)
{
    configData->custom.Motor1    = 0;
    configData->custom.Motor2    = 0;
    configData->custom.Motor3    = 0;
    configData->custom.Motor4    = 0;
    configData->custom.Motor5    = 0;
    configData->custom.Motor6    = 0;
    configData->custom.Motor7    = 0;
    configData->custom.Motor8    = 0;
    configData->custom.RevMotor1 = 0;
    configData->custom.RevMotor2 = 0;
    configData->custom.RevMotor3 = 0;
    configData->custom.RevMotor4 = 0;
    configData->custom.RevMotor5 = 0;
    configData->custom.RevMotor6 = 0;
    configData->custom.RevMotor7 = 0;
    configData->custom.RevMotor8 = 0;
    configData->custom.Servo1    = 0;
    configData->custom.Servo2    = 0;
    configData->custom.Servo3    = 0;
    configData->custom.Servo4    = 0;
    configData->custom.Servo5    = 0;
    configData->custom.Servo6    = 0;
    configData->custom.Servo7    = 0;
    configData->custom.Servo8    = 0;
}

/**
   Helper function to refresh the UI widget values
 */
void ConfigCustomWidget::refreshWidgetsValues(QString frameType)
{
    Q_ASSERT(m_aircraft);

    setupUI(frameType);

    UAVDataObject *system = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("SystemSettings")));
    Q_ASSERT(system);
    QPointer<UAVObjectField> field = system->getField(QString("AirframeType"));

    // Do not allow table edit until AirframeType == Custom
    // First save set AirframeType to 'Custom' and next modify.
    if (field->getValue().toString() != "Custom") {
        m_aircraft->customMixerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    } else {
        m_aircraft->customMixerTable->setEditTriggers(QAbstractItemView::AllEditTriggers);
    }

    UAVDataObject *mixer = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);

    getChannelDescriptions();

    QList<double> curveValues;
    getThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE1, &curveValues);

    // is at least one of the curve values != 0?
    if (isValidThrottleCurve(&curveValues)) {
        // yes, use the curve we just read from mixersettings
        m_aircraft->customThrottle1Curve->initCurve(&curveValues);
    } else {
        // no, init a straight curve
        m_aircraft->customThrottle1Curve->initLinearCurve(curveValues.count(), 1.0);
    }

    if (MixerSettings * mxr = qobject_cast<MixerSettings *>(mixer)) {
        MixerSettings::DataFields mixerSettingsData = mxr->getData();
        if (mixerSettingsData.Curve2Source == MixerSettings::CURVE2SOURCE_THROTTLE) {
            m_aircraft->customThrottle2Curve->setMixerType(MixerCurve::MIXERCURVE_THROTTLE);
        } else {
            m_aircraft->customThrottle2Curve->setMixerType(MixerCurve::MIXERCURVE_PITCH);
        }
    }

    // Setup all Throttle2 curves for all types of airframes
    getThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE2, &curveValues);

    if (isValidThrottleCurve(&curveValues)) {
        m_aircraft->customThrottle2Curve->initCurve(&curveValues);
    } else {
        m_aircraft->customThrottle2Curve->initLinearCurve(curveValues.count(), 1.0, m_aircraft->customThrottle2Curve->getMin());
    }

    // Update the mixer table:
    for (int channel = 0; channel < m_aircraft->customMixerTable->columnCount(); channel++) {
        UAVObjectField *field = mixer->getField(mixerTypes.at(channel));
        if (field) {
            QComboBox *q = (QComboBox *)m_aircraft->customMixerTable->cellWidget(0, channel);
            if (q) {
                QString s = field->getValue().toString();
                setComboCurrentIndex(q, q->findText(s));
            }

            m_aircraft->customMixerTable->item(1, channel)->setText(
                QString::number(getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_THROTTLECURVE1)));
            m_aircraft->customMixerTable->item(2, channel)->setText(
                QString::number(getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_THROTTLECURVE2)));
            m_aircraft->customMixerTable->item(3, channel)->setText(
                QString::number(getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL)));
            m_aircraft->customMixerTable->item(4, channel)->setText(
                QString::number(getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH)));
            m_aircraft->customMixerTable->item(5, channel)->setText(
                QString::number(getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW)));
        }
    }
}


/**
   Helper function to
 */
QString ConfigCustomWidget::updateConfigObjectsFromWidgets()
{
    UAVDataObject *system = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("SystemSettings")));

    Q_ASSERT(system);

    QPointer<UAVObjectField> field = system->getField(QString("AirframeType"));

    // Do not allow changes until AirframeType == Custom
    // If user want to save custom mixer : first set AirframeType to 'Custom' without changes and next modify.
    if (field->getValue().toString() == "Custom") {
        UAVDataObject *mixer = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("MixerSettings")));

        Q_ASSERT(mixer);

        setThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE1, m_aircraft->customThrottle1Curve->getCurve());
        setThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE2, m_aircraft->customThrottle2Curve->getCurve());

        GUIConfigDataUnion configData = getConfigData();
        resetActuators(&configData);

        // Update the table:
        for (int channel = 0; channel < (int)VehicleConfig::CHANNEL_NUMELEM; channel++) {
            QComboBox *q = (QComboBox *)m_aircraft->customMixerTable->cellWidget(0, channel);
            if (q->currentText() == "Disabled") {
                setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_DISABLED);
            } else if (q->currentText() == "Motor") {
                setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_MOTOR);
                if (configData.custom.Motor1 == 0) {
                    configData.custom.Motor1 = channel + 1;
                } else if (configData.custom.Motor2 == 0) {
                    configData.custom.Motor2 = channel + 1;
                } else if (configData.custom.Motor3 == 0) {
                    configData.custom.Motor3 = channel + 1;
                } else if (configData.custom.Motor4 == 0) {
                    configData.custom.Motor4 = channel + 1;
                } else if (configData.custom.Motor5 == 0) {
                    configData.custom.Motor5 = channel + 1;
                } else if (configData.custom.Motor6 == 0) {
                    configData.custom.Motor6 = channel + 1;
                } else if (configData.custom.Motor7 == 0) {
                    configData.custom.Motor7 = channel + 1;
                } else if (configData.custom.Motor8 == 0) {
                    configData.custom.Motor8 = channel + 1;
                }
            } else if (q->currentText() == "ReversableMotor") {
                setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_REVERSABLEMOTOR);
                if (configData.custom.RevMotor1 == 0) {
                    configData.custom.RevMotor1 = channel + 1;
                } else if (configData.custom.RevMotor2 == 0) {
                    configData.custom.RevMotor2 = channel + 1;
                } else if (configData.custom.RevMotor3 == 0) {
                    configData.custom.RevMotor3 = channel + 1;
                } else if (configData.custom.RevMotor4 == 0) {
                    configData.custom.RevMotor4 = channel + 1;
                } else if (configData.custom.RevMotor5 == 0) {
                    configData.custom.RevMotor5 = channel + 1;
                } else if (configData.custom.RevMotor6 == 0) {
                    configData.custom.RevMotor6 = channel;
                } else if (configData.custom.RevMotor7 == 0) {
                    configData.custom.RevMotor7 = channel;
                } else if (configData.custom.RevMotor8 == 0) {
                    configData.custom.RevMotor8 = channel;
                }
            } else if (q->currentText() == "Servo") {
                setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
                if (configData.custom.Servo1 == 0) {
                    configData.custom.Servo1 = channel + 1;
                } else if (configData.custom.Servo2 == 0) {
                    configData.custom.Servo2 = channel + 1;
                } else if (configData.custom.Servo3 == 0) {
                    configData.custom.Servo3 = channel + 1;
                } else if (configData.custom.Servo4 == 0) {
                    configData.custom.Servo4 = channel + 1;
                } else if (configData.custom.Servo5 == 0) {
                    configData.custom.Servo5 = channel + 1;
                } else if (configData.custom.Servo6 == 0) {
                    configData.custom.Servo6 = channel + 1;
                } else if (configData.custom.Servo7 == 0) {
                    configData.custom.Servo7 = channel + 1;
                } else if (configData.custom.Servo8 == 0) {
                    configData.custom.Servo8 = channel + 1;
                }
            } else if (q->currentText() == "CameraRoll") {
                setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_CAMERAROLL);
            } else if (q->currentText() == "CameraPitch") {
                setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_CAMERAPITCH);
            } else if (q->currentText() == "CameraYaw") {
                setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_CAMERAYAW);
            } else if (q->currentText() == "Accessory0") {
                setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_ACCESSORY0);
            } else if (q->currentText() == "Accessory1") {
                setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_ACCESSORY1);
            } else if (q->currentText() == "Accessory2") {
                setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_ACCESSORY2);
            } else if (q->currentText() == "Accessory3") {
                setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_ACCESSORY3);
            } else if (q->currentText() == "Accessory4") {
                setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_ACCESSORY4);
            } else if (q->currentText() == "Accessory5") {
                setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_ACCESSORY5);
            }
            setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_THROTTLECURVE1,
                                m_aircraft->customMixerTable->item(1, channel)->text().toDouble());
            setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_THROTTLECURVE2,
                                m_aircraft->customMixerTable->item(2, channel)->text().toDouble());
            setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL,
                                m_aircraft->customMixerTable->item(3, channel)->text().toDouble());
            setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH,
                                m_aircraft->customMixerTable->item(4, channel)->text().toDouble());
            setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW,
                                m_aircraft->customMixerTable->item(5, channel)->text().toDouble());
        }
        setConfigData(configData);
    }
    return "Custom";
}

/**
   This function displays text and color formatting in order to help the user understand what channels have not yet been configured.
 */
bool ConfigCustomWidget::throwConfigError(int numMotors)
{
    Q_UNUSED(numMotors);
    return false;
}

/**
   WHAT DOES THIS DO???
 */
void ConfigCustomWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    // Make the custom table columns autostretch:
    m_aircraft->customMixerTable->resizeColumnsToContents();
    int channelCount = (int)VehicleConfig::CHANNEL_NUMELEM;
    for (int i = 0; i < channelCount; i++) {
        m_aircraft->customMixerTable->setColumnWidth(i,
                                                     (m_aircraft->customMixerTable->width() - m_aircraft->customMixerTable->verticalHeader()->width())
                                                     / channelCount);
    }
}

/**
   Resize the GUI contents when the user changes the window size
 */
void ConfigCustomWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    // Make the custom table columns autostretch:
    m_aircraft->customMixerTable->resizeColumnsToContents();
    int channelCount = (int)VehicleConfig::CHANNEL_NUMELEM;
    for (int i = 0; i < channelCount; i++) {
        m_aircraft->customMixerTable->setColumnWidth(i,
                                                     (m_aircraft->customMixerTable->width() - m_aircraft->customMixerTable->verticalHeader()->width())
                                                     / channelCount);
    }
}

/**
   Helper delegate for the custom mixer editor table.
   Taken straight from Qt examples, thanks!
 */
SpinBoxDelegate::SpinBoxDelegate(QObject *parent) :
    QItemDelegate(parent)
{}

QWidget *SpinBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    QSpinBox *editor = new QSpinBox(parent);

    editor->setMinimum(-127);
    editor->setMaximum(127);

    return editor;
}

void SpinBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();

    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);

    spinBox->setValue(value);
}

void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);

    spinBox->interpretText();
    int value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}
