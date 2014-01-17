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

#define NEXT_EVENT     "next"
#define PREVIOUS_EVENT "previous"
#define ABORT_EVENT    "abort"
namespace OpenPilot {

ThermalCalibrationModel::ThermalCalibrationModel(QObject *parent) :
    WizardModel(parent)
{
    m_helper           = new ThermalCalibrationHelper();
    m_readyState       = new WizardState(tr("Start"), this),
    m_workingState     = new WizardState(NULL, this);

    m_saveSettingState = new WizardState(tr("Saving initial settings"), m_workingState);
    m_workingState->setInitialState(m_saveSettingState);

    m_setupState       = new WizardState(tr("Setup board for calibration"), m_workingState);

    m_acquisitionState = new WizardState(tr("*** Please Wait *** Samples acquisition, this can take several minutes"), m_workingState);
    m_calculateState   = new WizardState(tr("Calculate calibration matrix"), m_workingState);
    m_finalizeState    = new WizardState(tr("Completed"), m_workingState);

    m_abortState       = new WizardState("Canceled", this);

    setTransitions();

    connect(m_helper, SIGNAL(gradientChanged(float)), this, SLOT(setTemperatureGradient(float)));
    connect(m_helper, SIGNAL(temperatureChanged(float)), this, SLOT(setTemperature(float)));
    connect(m_helper, SIGNAL(processPercentageChanged(int)), this, SLOT(setProgress(int)));

    this->setInitialState(m_readyState);
    m_steps << m_readyState
            << m_setupState
            << m_acquisitionState << m_calculateState << m_finalizeState;
}
void ThermalCalibrationModel::init()
{
    setStartEnabled(true);
    setEndEnabled(false);
    setCancelEnabled(false);
    start();
    setTemperature(0);
    setTemperatureGradient(0);
    emit instructionsChanged(instructions());
}

void ThermalCalibrationModel::stepChanged(WizardState *state) {
    Q_UNUSED(state);
}

void ThermalCalibrationModel::setTransitions()
{
    m_readyState->addTransition(this, SIGNAL(next()), m_workingState);
    // handles board status save
    // Ready->WorkingState->saveSettings->setup
    m_saveSettingState->addTransition(new BoardStatusSaveTransition(m_helper, m_saveSettingState, m_setupState));
    // board setup
    // setup
    m_setupState->addTransition(new BoardSetupTransition(m_helper, m_setupState, m_acquisitionState));

    // acquisition -revertSettings-> calculation
    // m_acquisitionState->addTransition(this,SIGNAL(next()),m_calculateState);
    // revert settings after acquisition is completed
    // m_acquisitionState->addTransition(new BoardStatusRestoreTransition(m_helper, m_acquisitionState, m_calculateState));
    m_acquisitionState->addTransition(new DataAcquisitionTransition(m_helper, m_acquisitionState, m_calculateState));
    m_workingState->addTransition(m_helper, SIGNAL(abort()), m_abortState) ;
    m_calculateState->addTransition(new BoardStatusRestoreTransition(m_helper, m_calculateState, m_finalizeState));

    m_abortState->addTransition(new BoardStatusRestoreTransition(m_helper, m_abortState, m_readyState));
    m_finalizeState->addTransition(this, SIGNAL(next()), m_readyState);
    // Ready
}
}
