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
#include "../wizardstate.h"
#include "../wizardmodel.h"

#include <QObject>
#include <QState>
#include <QStateMachine>

namespace OpenPilot {
class ThermalCalibrationModel : public WizardModel {
    Q_OBJECT Q_PROPERTY(bool startEnable READ startEnabled NOTIFY startEnabledChanged)
    Q_PROPERTY(bool endEnable READ endEnabled NOTIFY endEnabledChanged)
    Q_PROPERTY(bool cancelEnable READ cancelEnabled NOTIFY cancelEnabledChanged)
    Q_PROPERTY(float temperature READ temperature NOTIFY temperatureChanged)
    Q_PROPERTY(float temperatureGradient READ temperatureGradient NOTIFY temperatureGradientChanged)
    Q_PROPERTY(float temperatureRange READ temperatureRange NOTIFY temperatureRangeChanged)
    Q_PROPERTY(int progress READ progress WRITE setProgress NOTIFY progressChanged)
    Q_PROPERTY(int progressMax READ progressMax WRITE setProgressMax NOTIFY progressMaxChanged)

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

    float temperature()
    {
        return m_helper->temperature();
    }

    float temperatureGradient()
    {
        return m_helper->gradient();
    }

    float temperatureRange()
    {
        return m_helper->range();
    }

    bool dirty()
    {
        return m_dirty;
    }


public slots:
    void setTemperature(float temperature)
    {
        emit temperatureChanged(temperature);
    }

    void setTemperatureGradient(float temperatureGradient)
    {
        emit temperatureGradientChanged(temperatureGradient);
    }

    void setTemperatureRange(float temperatureRange)
    {
        emit temperatureRangeChanged(temperatureRange);
    }

    int progress()
    {
        return m_progress;
    }

    void setProgress(int value)
    {
        m_progress = value;
        emit progressChanged(value);
    }

    int progressMax()
    {
        return m_progressMax;
    }

    void setProgressMax(int value)
    {
        m_progressMax = value;
        emit progressMaxChanged(value);
    }

private:
    QScopedPointer<ThermalCalibrationHelper> m_helper;

    bool m_startEnabled;
    bool m_cancelEnabled;
    bool m_endEnabled;
    bool m_dirty;

    int m_progress;
    int m_progressMax;

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
    // Restore initial settings after acquisition
    WizardState *m_restoreState;
    // Calculate calibration parameters
    WizardState *m_calculateState;
    // Save calibration and restore board settings
    WizardState *m_finalizeState;
    // revert board settings if something goes wrong
    WizardState *m_abortState;
    // just the same as ready state, but it is reached after having completed the calibration
    WizardState *m_completedState;
    void setTransitions();

signals:
    void startEnabledChanged(bool state);
    void endEnabledChanged(bool state);
    void cancelEnabledChanged(bool state);

    void wizardStarted();
    void wizardStopped();

    void temperatureChanged(float temperature);
    void temperatureGradientChanged(float temperatureGradient);
    void temperatureRangeChanged(float temperatureRange);

    void progressChanged(int value);
    void progressMaxChanged(int value);

    void next();
    void previous();
    void abort();

public slots:
    void stepChanged(WizardState *state);
    void init();
    void btnStart()
    {
        init();
        emit next();
    }
    void btnEnd()
    {
        m_helper->stopAcquisition();
    }
    void btnAbort()
    {
        emit abort();
    }
    void startWizard()
    {
        m_dirty = false;
        setStartEnabled(false);
        setEndEnabled(true);
        setCancelEnabled(true);
        wizardStarted();
    }
    void stopWizard()
    {
        m_dirty = m_helper->calibrationSuccessful();
        setStartEnabled(true);
        setEndEnabled(false);
        setCancelEnabled(false);
        wizardStopped();
    }
    void save()
    {
        if (m_dirty) {
            m_dirty = false;
            m_helper->copyResultToSettings();
        }
    }
};
}

#endif // THERMALCALIBRATIONMODEL_H
