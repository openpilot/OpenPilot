/**
 ******************************************************************************
 *
 * @file       thermalcalibrationmodel.cpp
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

#include "thermalcalibrationmodel.h"
#include "settingshandlingtransitions.h"
#include "boardsetuptransition.h"
#include "dataacquisitiontransition.h"
#include "compensationcalculationtransition.h"

namespace OpenPilot {
ThermalCalibrationModel::ThermalCalibrationModel(QObject *parent) :
    WizardModel(parent),
    m_startEnabled(false),
    m_cancelEnabled(false),
    m_endEnabled(false),
    m_initDone(false)
{
    m_helper.reset(new ThermalCalibrationHelper());

    m_readyState       = new WizardState("Ready", this),
    m_workingState     = new WizardState("Working", this);

    m_saveSettingState = new WizardState("Storing Settings", m_workingState);
    m_workingState->setInitialState(m_saveSettingState);

    m_setupState       = new WizardState("SetupBoard", m_workingState);

    m_acquisitionState = new WizardState("Acquiring", m_workingState);
    m_restoreState     = new WizardState("Restoring Settings", m_workingState);
    m_calculateState   = new WizardState("Calculating", m_workingState);

    m_abortState       = new WizardState("Canceled", this);

    m_completedState   = new WizardState("Completed", this);

    setTransitions();

    connect(m_helper.data(), SIGNAL(temperatureChanged(float)), this, SLOT(setTemperature(float)));
    connect(m_helper.data(), SIGNAL(temperatureGradientChanged(float)), this, SLOT(setTemperatureGradient(float)));
    connect(m_helper.data(), SIGNAL(progressChanged(int)), this, SLOT(setProgress(int)));
    connect(m_helper.data(), SIGNAL(instructionsAdded(QString, WizardModel::MessageType)), this, SLOT(addInstructions(QString, WizardModel::MessageType)));
    connect(m_readyState, SIGNAL(entered()), this, SLOT(wizardReady()));
    connect(m_readyState, SIGNAL(exited()), this, SLOT(wizardStarted()));
    connect(m_completedState, SIGNAL(entered()), this, SLOT(wizardReady()));
    connect(m_completedState, SIGNAL(exited()), this, SLOT(wizardStarted()));

    setInitialState(m_readyState);

    m_steps << m_readyState << m_saveSettingState << m_setupState << m_acquisitionState << m_restoreState << m_calculateState;
}

void ThermalCalibrationModel::init()
{
    if (!m_initDone) {
        m_initDone = true;
        setStartEnabled(true);
        setEndEnabled(false);
        setCancelEnabled(false);
        start();
        setTemperature(0);
        setTemperatureGradient(0);
    }
}

void ThermalCalibrationModel::stepChanged(WizardState *state)
{
    Q_UNUSED(state);
}

void ThermalCalibrationModel::setTransitions()
{
    m_readyState->addTransition(this, SIGNAL(next()), m_workingState);
    m_readyState->assignProperty(this, "progress", 0);

    m_completedState->addTransition(this, SIGNAL(next()), m_workingState);
    m_completedState->assignProperty(this, "progress", 100);

    // handles board initial status save
    // Ready->WorkingState->saveSettings->setup
    m_saveSettingState->addTransition(new BoardStatusSaveTransition(m_helper.data(), m_saveSettingState, m_setupState));
    // board setup
    // setup
    m_setupState->addTransition(new BoardSetupTransition(m_helper.data(), m_setupState, m_acquisitionState));

    // acquisition>revertSettings>calculation(save state)
    // revert settings after acquisition is completed
    m_acquisitionState->addTransition(new DataAcquisitionTransition(m_helper.data(), m_acquisitionState, m_restoreState));
    m_restoreState->addTransition(new BoardStatusRestoreTransition(m_helper.data(), m_restoreState, m_calculateState));
    m_calculateState->addTransition(new CompensationCalculationTransition(m_helper.data(), m_calculateState, m_completedState));

    // abort causes initial settings to be restored and acquisition stopped.
    m_abortState->addTransition(new BoardStatusRestoreTransition(m_helper.data(), m_abortState, m_readyState));
    m_workingState->addTransition(this, SIGNAL(abort()), m_abortState);
    // Ready
}
}
