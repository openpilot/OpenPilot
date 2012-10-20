/**
 ******************************************************************************
 *
 * @file       enums.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup
 * @{
 * @brief
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
#ifndef ENUMS_H
#define ENUMS_H

namespace uploader
{
    typedef enum { IAP_STATE_READY, IAP_STATE_STEP_1, IAP_STATE_STEP_2, IAP_STEP_RESET, IAP_STATE_BOOTLOADER} IAPStep;
    typedef enum { WAITING_DISCONNECT, WAITING_CONNECT, JUMP_TO_BL, LOADING_FW, UPLOADING_FW, UPLOADING_DESC, BOOTING, SUCCESS, FAILURE} AutoUpdateStep;
}
#endif // ENUMS_H
