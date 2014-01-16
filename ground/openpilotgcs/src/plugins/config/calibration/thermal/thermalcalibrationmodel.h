/**
 ******************************************************************************
 *
 * @file       thermalcalibrationmodel.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 *
 * @brief      ThermalCalibrationModel
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

#ifndef THERMALCALIBRATIONMODEL_H
#define THERMALCALIBRATIONMODEL_H

#include "thermalcalibrationhelper.h"


#include <QObject>
#include <QState>
#include <QStateMachine>
#include "../wizardstate.h"
#include "../wizardmodel.h"

class ThermalCalibrationModel : public WizardModel {
    Q_PROPERTY(bool startEnable READ startEnabled NOTIFY startEnabledChanged)
    Q_PROPERTY(bool endEnable READ endEnabled NOTIFY endEnabledChanged)
    Q_PROPERTY(bool cancelEnable READ cancelEnabled NOTIFY cancelEnabledChanged)

    Q_PROPERTY(QString temperature READ temperature NOTIFY temperatureChanged)
    Q_PROPERTY(QString temperatureGradient READ temperatureGradient NOTIFY temperatureGradientChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_OBJECT
public:
    explicit ThermalCalibrationModel(QObject *parent = 0);

    bool startEnabled()
    {
        return m_startEnabled;
    }
    bool endEnabled()
    {
        return m_endEnabled;
    }
    bool cancelEnabled()
    {
        return m_cancelEnabled;
    }

    void setStartEnabled(bool status)
    {
        if (m_startEnabled != status) {
            m_startEnabled = status;
            emit startEnabledChanged(status);
        }
    }

    void setEndEnabled(bool status)
    {
        if (m_endEnabled != status) {
            m_endEnabled = status;
            emit endEnabledChanged(status);
        }
    }
    void setCancelEnabled(bool status)
    {
        if (m_cancelEnabled != status) {
            m_cancelEnabled = status;
            emit cancelEnabledChanged(status);
        }
    }


public slots:
    int progress()
    {
        return m_progress;
    }

    QString temperature()
    {
        return m_temperature;
    }

    QString temperatureGradient()
    {
        return m_temperatureGradient;
    }

    void setTemperature(float status)
    {
        QString tmp = QString("%1").arg(status, 5, 'f', 2);

        if (m_temperature != tmp) {
            m_temperature = tmp;
            emit temperatureChanged(tmp);
        }
    }
    void setTemperatureGradient(float status)
    {
        QString tmp = QString("%1 %2").arg(status, 5, 'f', 2);

        if (m_temperatureGradient != tmp) {
            m_temperatureGradient = tmp;
            emit temperatureGradientChanged(tmp);
        }
    }
    void setProgress(int status)
    {
        m_progress = status;
        emit progressChanged(status);
        if (this->currentState()) {
            setInstructions(this->currentState()->stepName());
        }
    }

private:
    bool m_startEnabled;
    bool m_cancelEnabled;
    bool m_endEnabled;
    int m_progress;
    QString m_temperature;
    QString m_temperatureGradient;

    ThermalCalibrationHelper *m_helper;

    // Start from here
    WizardState *m_readyState;
    // this act as top level container for calibration state
    // to share common exit signals to abortState
    WizardState *m_workingState;
    // Save settings
    WizardState *m_saveSettingState;
    // Setup board for calibration
    WizardState *m_setupState;
    // Acquire samples
    WizardState *m_acquisitionState;
    // Calculate calibration parameters
    WizardState *m_calculateState;
    // Save calibration and restore board settings
    WizardState *m_finalizeState;
    // revert board settings if something goes wrong
    WizardState *m_abortState;

    void setTransitions();


signals:
    void startEnabledChanged(bool state);
    void endEnabledChanged(bool state);
    void cancelEnabledChanged(bool state);

    void temperatureChanged(QString status);
    void temperatureGradientChanged(QString status);
    void progressChanged(int value);

    void next();
    void previous();
    void abort();
public slots:
    void stepChanged(WizardState *state);
    void init();
    void btnStart()
    {
        emit next();
    }

    void btnEnd()
    {
        // emit previous();
        m_helper->endAcquisition();
    }

    void btnAbort()
    {
        emit abort();
    }
};


#endif // THERMALCALIBRATIONMODEL_H
