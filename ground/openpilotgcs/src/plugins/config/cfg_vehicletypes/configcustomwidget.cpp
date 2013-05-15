/**
 ******************************************************************************
 *
 * @file       configmultirotorwidget.cpp
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief ccpm configuration panel
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
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QBrush>
#include <math.h>
#include <QMessageBox>

QStringList ConfigCustomWidget::getChannelDescriptions()
{
    QStringList channelDesc;
    for (int i = 0; i < (int) VehicleConfig::CHANNEL_NUMELEM; i++) {
        channelDesc.append(QString("-"));
    }
    return channelDesc;
}

ConfigCustomWidget::ConfigCustomWidget(QWidget *parent) :
        VehicleConfig(parent), m_aircraft(new Ui_CustomConfigWidget())
{
    m_aircraft->setupUi(this);

    // Put combo boxes in line one of the custom mixer table:
    UAVDataObject *mixer = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);

    UAVObjectField* field = mixer->getField(QString("Mixer1Type"));
    QStringList list = field->getOptions();
    for (int i = 0; i < (int) VehicleConfig::CHANNEL_NUMELEM; i++) {
        QComboBox* qb = new QComboBox(m_aircraft->customMixerTable);
        qb->addItems(list);
        m_aircraft->customMixerTable->setCellWidget(0, i, qb);
    }

    SpinBoxDelegate *sbd = new SpinBoxDelegate();
    for (int i = 1; i < (int) VehicleConfig::CHANNEL_NUMELEM; i++) {
        m_aircraft->customMixerTable->setItemDelegateForRow(i, sbd);
    }
}

ConfigCustomWidget::~ConfigCustomWidget()
{
    delete m_aircraft;
}

void ConfigCustomWidget::setupUI(QString frameType)
{
    Q_ASSERT(m_aircraft);
}

void ConfigCustomWidget::registerWidgets(ConfigTaskWidget &parent) {
    parent.addWidget(m_aircraft->customMixerTable);
    parent.addWidget(m_aircraft->customThrottle1Curve->getCurveWidget());
    parent.addWidget(m_aircraft->customThrottle1Curve);
    parent.addWidget(m_aircraft->customThrottle2Curve->getCurveWidget());
    parent.addWidget(m_aircraft->customThrottle2Curve);
}

void ConfigCustomWidget::resetActuators(GUIConfigDataUnion *configData)
{
}

/**
 Helper function to refresh the UI widget values
 */
void ConfigCustomWidget::refreshWidgetsValues(QString frameType)
{
    Q_ASSERT(m_aircraft);

    setupUI(frameType);

    UAVDataObject *mixer = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);

    QList<double> curveValues;
    getThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE1, &curveValues);

    // is at least one of the curve values != 0?
    if (isValidThrottleCurve(&curveValues)) {
        // yes, use the curve we just read from mixersettings
        m_aircraft->customThrottle1Curve->initCurve(&curveValues);
    }
    else {
        // no, init a straight curve
        m_aircraft->customThrottle1Curve->initLinearCurve(curveValues.count(), 1.0);
    }

    if (MixerSettings *mxr = qobject_cast<MixerSettings *>(mixer)) {
        MixerSettings::DataFields mixerSettingsData = mxr->getData();
        if (mixerSettingsData.Curve2Source == MixerSettings::CURVE2SOURCE_THROTTLE)
            m_aircraft->customThrottle2Curve->setMixerType(MixerCurve::MIXERCURVE_THROTTLE);
        else {
            m_aircraft->customThrottle2Curve->setMixerType(MixerCurve::MIXERCURVE_PITCH);
        }
    }

    // Setup all Throttle2 curves for all types of airframes
    getThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE2, &curveValues);

    if (isValidThrottleCurve(&curveValues)) {
        m_aircraft->customThrottle2Curve->initCurve(&curveValues);
    }
    else {
        m_aircraft->customThrottle2Curve->initLinearCurve(curveValues.count(), 1.0, m_aircraft->customThrottle2Curve->getMin());
    }

    // Update the mixer table:
    for (int channel = 0; channel < m_aircraft->customMixerTable->columnCount(); channel++) {
        UAVObjectField* field = mixer->getField(mixerTypes.at(channel));
        if (field) {
            QComboBox* q = (QComboBox*) m_aircraft->customMixerTable->cellWidget(0, channel);
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
    UAVDataObject *mixer = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);

    setThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE1, m_aircraft->customThrottle1Curve->getCurve());
    setThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE2, m_aircraft->customThrottle2Curve->getCurve());

    // Update the table:
    for (int channel = 0; channel < (int) VehicleConfig::CHANNEL_NUMELEM; channel++) {
        QComboBox* q = (QComboBox*) m_aircraft->customMixerTable->cellWidget(0, channel);
        if (q->currentText() == "Disabled") {
            setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_DISABLED);
        } else if (q->currentText() == "Motor") {
            setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_MOTOR);
        } else if (q->currentText() == "Servo") {
            setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
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

    return "Custom";
}

/**
 This function displays text and color formatting in order to help the user understand what channels have not yet been configured.
 */
bool ConfigCustomWidget::throwConfigError(int numMotors)
{    
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
    int channelCount = (int) VehicleConfig::CHANNEL_NUMELEM;
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
    int channelCount = (int) VehicleConfig::CHANNEL_NUMELEM;
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
{
}

QWidget *SpinBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    QSpinBox *editor = new QSpinBox(parent);
    editor->setMinimum(-127);
    editor->setMaximum(127);

    return editor;
}

void SpinBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();

    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(value);
}

void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}
