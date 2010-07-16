/**
 ******************************************************************************
 *
 * @file       pjrc_rawhid.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup RawHIDPlugin Raw HID Plugin
 * @{
 * @brief Impliments a HID USB connection to the flight hardware as a QIODevice
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

#ifndef PJRC_RAWHID_H
#define PJRC_RAWHID_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <QDebug>

class pjrc_rawhid
{
public:
    pjrc_rawhid();
    int open(int max, int vid, int pid, int usage_page, int usage);
    int receive(int num, void *buf, int len, int timeout);
    void close(int num);
    int send(int num, void *buf, int len, int timeout);
    int getserial(int num, char *buf);

private:

};

#endif // PJRC_RAWHID_H
