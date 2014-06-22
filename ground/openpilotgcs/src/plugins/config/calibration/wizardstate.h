/**
 ******************************************************************************
 *
 * @file       wizardstate.c
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
#ifndef WIZARDSTATE_H
#define WIZARDSTATE_H

#include <QState>

class WizardState : public QState {
    Q_OBJECT Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(bool isDone READ isDone NOTIFY isDoneChanged)
    Q_PROPERTY(qint8 completion READ completion NOTIFY completionChanged)
    Q_PROPERTY(QString stepName READ stepName)

public:
    explicit WizardState(QString name, QState *parent = 0);

    bool isActive()
    {
        return m_active;
    }

    bool isDone()
    {
        return m_done;
    }

    qint8 completion()
    {
        return m_completion;
    }

    QString stepName()
    {
        return m_stepName;
    }

    void setCompletion(qint8 completion);
    virtual void onEntry(QEvent *event) Q_DECL_OVERRIDE;
    virtual void onExit(QEvent *event) Q_DECL_OVERRIDE;

signals:
    void isActiveChanged();
    void isDoneChanged();
    void completionChanged();

public slots:
    void clean();

private:
    void setIsDone(bool done);
    bool m_done;
    bool m_active;
    qint8 m_completion;
    QString m_stepName;
};

#endif // WIZARDSTATE_H
