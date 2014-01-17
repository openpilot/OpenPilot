/**
 ******************************************************************************
 *
 * @file       thermalcalibrationtransitions.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 *
 * @brief      State transitions used to implement thermal calibration
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

#ifndef DATAACQUISITIONTRANSITION_H
#define DATAACQUISITIONTRANSITION_H

#include <QSignalTransition>
#include <QEventTransition>

#include "thermalcalibrationhelper.h"

class DataAcquisitionTransition : public QSignalTransition {
    Q_OBJECT
public:
    DataAcquisitionTransition(ThermalCalibrationHelper *helper, QState *currentState, QState *targetState)
        : QSignalTransition(helper, SIGNAL(collectionCompleted())),
        m_helper(helper)
    {
        QObject::connect(currentState, SIGNAL(entered()), this, SLOT(enterState()));

        setTargetState(targetState);
    }

    virtual void onTransition(QEvent *e)
    {
        Q_UNUSED(e);
        qDebug() << "DataAcquisitionTransition::collectionCompleted";
    }

    virtual void exit(int __status){
        Q_UNUSED(__status);
        m_helper->endAcquisition();
    }

public slots:
    void enterState()
    {
        qDebug() << "DataAcquisitionTransition::enterStatus";
        m_helper->initAcquisition();
    }
private:
    ThermalCalibrationHelper *m_helper;
};

#endif // DATAACQUISITIONTRANSITION_H
