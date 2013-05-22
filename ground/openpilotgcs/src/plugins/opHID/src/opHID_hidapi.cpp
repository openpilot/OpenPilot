/**
 ******************************************************************************
 *
 * @file       rawhid.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup opHIDPlugin OpenPilot HID Plugin
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

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "opHID_const.h"
#include "opHID_hidapi.h"

/**
* \brief Constructor
*
* \note
*
*/
opHID_hidapi::opHID_hidapi()
{
    OPHID_TRACE("IN");

    handle = NULL;

    // Make sure hidapi lib is ready
    if (hid_init())
        OPHID_ERROR("Lib initialization (hidpai).");

    OPHID_TRACE("OUT");
}


/**
* \brief Destructor
*
* \note This does not handle the cleanup of hidapi lib
*
*/
opHID_hidapi::~opHID_hidapi()
{
    OPHID_TRACE("IN");

    OPHID_TRACE("OUT");
}


/**
* \brief Enumerate the list of HID device with our vendor id
*
* \note Why don't we use the one from within the hidapi directly
*      in caller? because later we will do more parsing herer.
*      WARNING: our vendor id is harcoded here (not good idea).
*
* \param[out] current_device_pptr Pointer to the list of device
* \param[out] devices_found Number of devices found.
* \return error.
* \retval 0 on success.
*/
int opHID_hidapi::enumerate(struct hid_device_info **current_device_pptr, int *devices_found)
{
    int retry = 5;
    *devices_found = 0;

    struct hid_device_info *current_device_ptr = NULL;

    OPHID_TRACE("IN");

    while(retry--)
    {
        // Enumerate	
        *current_device_pptr = hid_enumerate(USB_VID, 0x0);

        // Display the list of devices found (for debug)
        current_device_ptr = *current_device_pptr;

        while (current_device_ptr) {
            OPHID_DEBUG("HID Device Found");
            OPHID_DEBUG("  type:............VID(%04hx).PID(%04hx)", current_device_ptr->vendor_id, current_device_ptr->product_id);
            OPHID_DEBUG("  path:............%s", current_device_ptr->path);
            OPHID_DEBUG("  Release:.........%hx", current_device_ptr->release_number);
            OPHID_DEBUG("  Interface:.......%d", current_device_ptr->interface_number);
            current_device_ptr = current_device_ptr->next;
            (*devices_found)++;
        }

        if (*devices_found)
            break;
    }

    OPHID_TRACE("OUT");
    return OPHID_NO_ERROR;
}


/**
* \brief Open HID device using hidapi library
*
* \note This function does \b not support opening multiple devices at once.
*
* \param[in] vid USB vendor id of the device to open (-1 for any).
* \param[in] pid USB product id of the device to open (-1 for any).
* \return Number of opened device.
* \retval 0 or 1.
*/
int opHID_hidapi::open(int max, int vid, int pid, int usage_page, int usage)
{
    int devices_found = false;
    struct hid_device_info *current_device_ptr = NULL;
    struct hid_device_info *last_device_ptr = NULL;
    struct hid_device_info **current_device_pptr = &current_device_ptr;

    OPHID_TRACE("IN");

    OPHID_DEBUG("max: %d, vid: 0x%X, pid: 0x%X, usage_page: %d, usage: %d.", max, vid, pid, usage_page, usage);

    if (handle)
    {
        OPHID_WARNING("HID device seems already open.");
    }

    // This is a hack to prevent changing all the callers (for now)
    if (vid == -1)
        vid = 0;
    if (pid == -1)
        pid = 0;

    // If caller knows which one to look for open it right away
    if (vid != 0 && pid != 0)
    {
        handle = hid_open(vid, pid, NULL);

        if (!handle)
        {
            OPHID_ERROR("Unable to open device.");
            devices_found = false;
        }
        else
        {
            OPHID_DEBUG("HID Device Found");
            OPHID_DEBUG("  type:............VID(%04hx).PID(%04hx)", vid, pid);
            devices_found = true;
        }
    }
    else
    {
        // Get the list of available hid devices
        if (enumerate(current_device_pptr, &devices_found) != OPHID_NO_ERROR)
        {
            OPHID_ERROR("Error during enumeration");
            return 0;
        }

        if (devices_found)
        {
            // Look for the last one in the list
            // WARNING: for now this prevent to have devices chained
            last_device_ptr = current_device_ptr;
            while (last_device_ptr->next)
                last_device_ptr = last_device_ptr->next;

            OPHID_DEBUG("Opening device VID(%04hx).PID(%04hx)", 
                      last_device_ptr->vendor_id, 
                      last_device_ptr->product_id);
            
            handle = hid_open(last_device_ptr->vendor_id,
                              last_device_ptr->product_id, 
                              NULL);

            hid_free_enumeration(current_device_ptr);

            if (!handle)
            {
                OPHID_ERROR("Unable to open device.");    
                devices_found = false;
            }

        }
        else
        {
            OPHID_WARNING("Unable to find any HID device.");
        }
    }

    OPHID_DEBUG("Found %d devices", devices_found);

    OPHID_TRACE("OUT");

    return  devices_found;
}


/**
* \brief Read an Input report from a HID device.
*
* \note This function does \b not block for now. 
*      Tests show that it does not need to.
*
* \param[in] num Id of the device to receive packet (NOT supported).
* \param[in] buf Pointer to the bufer to write the received packet to.
* \param[in] len Size of the buffer.
* \param[in] timeout Not supported.
* \return Number of bytes received, or -1 on error.
* \retval -1 for error or bytes received.
*/
int opHID_hidapi::receive(int num, void *buf, int len, int timeout)
{
    Q_UNUSED(num);
    Q_UNUSED(timeout);

    int bytes_read = 0;

    if (!buf)
    {
        OPHID_ERROR("Unexpected parameter value (ptr).");
        return OPHID_ERROR_POINTER;
    }

    if (!len)
    {
        OPHID_ERROR("Unexpected parameter value (incorrect lenght).");
        return OPHID_ERROR_PARAMETER;
    }

    if (handle == NULL)
    {
        OPHID_ERROR("Handle invalid.");
        return OPHID_ERROR_HANDLE;
    }

    hid_read_Mtx.lock();
    bytes_read = hid_read(handle, (unsigned char*)buf, len);
    hid_read_Mtx.unlock();

    // hidapi lib does not expose the libusb errors.
    if (bytes_read == -1)
    {
        OPHID_ERROR("hidapi: %d", bytes_read);
    }
    
    return bytes_read;
}

/**
* \brief Write an Output report to a HID device.
*
* \note timeout is 1000ms for control transfer and
*      1000 ms for interrupt transfer.
*
* \param[in] num Id of the device to receive packet (NOT supported).
* \param[in] buf Pointer to the bufer to send.
* \param[in] len Size of the buffer.
* \param[in] timeout (not supported).
* \return Number of bytes received, or -1 on error.
* \retval -1 for error or bytes received.
*/
int opHID_hidapi::send(int num, void *buf, int len, int timeout)
{
    Q_UNUSED(num);
    Q_UNUSED(timeout);

    int bytes_written = 0;
    int retry = 5;

    if (!buf)
    {
	OPHID_ERROR("Unexpected parameter value (ptr).");
        return OPHID_ERROR_POINTER;
    }

    if (!len)
    {
        OPHID_ERROR("Unexpected parameter value (incorrect lenght).");
        return OPHID_ERROR_PARAMETER;
    }

    if (handle == NULL)
    {
        OPHID_ERROR("Handle invalid.");
        return OPHID_ERROR_HANDLE;
    }

    // hidapi has a timeout hardcoded to 1000ms, retry 5 times
    while(retry--)
    {
        hid_write_Mtx.lock();
        bytes_written = hid_write(handle, (const unsigned char*)buf, len);
        hid_write_Mtx.unlock();
        if (bytes_written >= 0)
        {
            break;
        }
    }

    // hidapi lib does not expose the libusb errors.
    if (bytes_written < 0)
    {
        OPHID_ERROR("hidapi: %d", bytes_written);
    }

    return bytes_written;
}


/**
* \brief Return the serial number of a device.
*
* \note This function does \b not handle multiple
*      HID devices. Only the serial number of the 
*      current HID device will supported.
*
* \param[in] num Id of the device to request SN (NOT supported).
* \return Serial number
* \retval string
*/
QString opHID_hidapi::getserial(int num)
{
    Q_UNUSED(num);

    OPHID_TRACE("IN");

    wchar_t buf[USB_MAX_STRING_SIZE];
    
    if (handle == NULL)
    {
        OPHID_ERROR("Handle invalid.");
        return QString("");
    }

    if (hid_get_serial_number_string(handle, buf, USB_MAX_STRING_SIZE) < 0)
    {
        OPHID_ERROR("Unable to read serial number string.");
        return QString("");
    }
    
    OPHID_TRACE("OUT");
    return QString().fromWCharArray(buf);
}


/**
* \brief Close a HID device
*
* \note This function does \b not handle multiple
*      HID devices currently. 
*
* \param[in] num Id of the device to close (NOT supported).
*/
void opHID_hidapi::close(int num)
{
    Q_UNUSED(num);

    OPHID_TRACE("IN");

    if (handle)
       hid_close(handle);

    handle = NULL;

    OPHID_TRACE("OUT");
}

