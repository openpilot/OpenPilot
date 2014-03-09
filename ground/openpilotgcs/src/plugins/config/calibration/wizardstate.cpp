/**
 ******************************************************************************
 *
 * @file       wizardstate.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 *
 * @brief      A fsm state used in wizard like UI
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
#include "wizardstate.h"
#include "QDebug"
WizardState::WizardState(QString name, QState *parent) :
    QState(parent)
{
    m_stepName = name;
    m_done     = false;
    m_active   = false;
}

void WizardState::setCompletion(qint8 completion)
{
    m_completion = completion;
    emit completionChanged();
}

void WizardState::setStepName(QString name)
{
    m_stepName = name;
    emit stepNameChanged();
}

void WizardState::onEntry(QEvent *event)
{
    Q_UNUSED(event);
    m_active = true;
    emit isActiveChanged();
}

void WizardState::onExit(QEvent *event)
{
    Q_UNUSED(event);
    m_active = false;
    setIsDone(true);
    emit isActiveChanged();
}

void WizardState::clean()
{
    setIsDone(false);
}

void WizardState::setIsDone(bool done)
{
    m_done = done;
    emit isDoneChanged();
}
