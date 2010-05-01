/**
 ******************************************************************************
 *
 * @file       airspeedgadgetconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Airspeed Plugin Gadget configuration
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   Airspeed
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

#ifndef AIRSPEEDGADGETCONFIGURATION_H
#define AIRSPEEDGADGETCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>

using namespace Core;

class AirspeedGadgetConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT
public:
    explicit AirspeedGadgetConfiguration(QString classId, const QByteArray &state = 0, QObject *parent = 0);

    //set dial configuration functions
    void setDialFile(QString dialFile){m_defaultDial=dialFile;}


    //get port configuration functions
    QString dialFile() {return m_defaultDial;}

    QByteArray saveState() const;
    IUAVGadgetConfiguration *clone();

private:
    QString m_defaultDial;

};

#endif // AIRSPEEDGADGETCONFIGURATION_H
