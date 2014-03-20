/**
 ******************************************************************************
 *
 * @file       wizardmodel.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 *
 * @brief      A fsm state machine model used in wizard like UIs
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
#ifndef WIZARDMODEL_H
#define WIZARDMODEL_H

#include <QStateMachine>
#include <QQmlListProperty>
#include "wizardstate.h"
#include <accelsensor.h>

class WizardModel : public QStateMachine {
    Q_OBJECT Q_PROPERTY(QQmlListProperty<QObject> steps READ steps CONSTANT)
    Q_PROPERTY(QString instructions READ instructions NOTIFY instructionsChanged)
    Q_PROPERTY(WizardState * currentState READ currentState NOTIFY currentStateChanged)
public:
    explicit WizardModel(QObject *parent = 0);

    QQmlListProperty<QObject> steps()
    {
        return QQmlListProperty<QObject>(this, m_steps);
    }

    QString instructions()
    {
        return m_instructions;
    }

    void setInstructions(QString instructions)
    {
        m_instructions = instructions;
        emit instructionsChanged(instructions);
    }
    WizardState *currentState();
protected:
    QList<QObject *> m_steps;
private:
    QString m_instructions;
signals:
    void instructionsChanged(QString status);
    void currentStateChanged(WizardState *status);
public slots:
};

#endif // WIZARDMODEL_H
