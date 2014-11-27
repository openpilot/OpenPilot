/**
 ******************************************************************************
 *
 * @file       common.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup Uploader Serial and USB Uploader Plugin
 * @{
 * @brief The USB and Serial protocol uploader plugin
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
 */#ifndef COMMON_H
#define COMMON_H

enum decodeState_ {
    decode_len1_e = 0,
    decode_seqNo_e,
    decode_data_e,
    decode_crc1_e,
    decode_crc2_e,
    decode_idle_e
};
enum ReceiveState {
    state_escaped_e = 0,
    state_unescaped_e
};


#endif // COMMON_H
