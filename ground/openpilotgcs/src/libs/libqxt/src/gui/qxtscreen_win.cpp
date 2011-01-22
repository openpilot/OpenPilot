/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtGui module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/
#include "qxtscreen_p.h"
#include <qt_windows.h>

void QxtScreenPrivate::init_sys()
{
    DISPLAY_DEVICE displayDevice;
    ::ZeroMemory(&displayDevice, sizeof(displayDevice));
    displayDevice.cb = sizeof(displayDevice);

    if (::EnumDisplayDevices(NULL, screen, &displayDevice, 0))
    {
        DEVMODE devMode;
        ::ZeroMemory(&devMode, sizeof(devMode));
        devMode.dmSize = sizeof(devMode);

        // current resolution & rate
        if (!currReso.isValid() || currRate < 0 || currDepth < 0)
        {
            if (::EnumDisplaySettings(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &devMode))
            {
                currReso = QSize(devMode.dmPelsWidth, devMode.dmPelsHeight);
                currRate = devMode.dmDisplayFrequency;
                currDepth = devMode.dmBitsPerPel;
            }
        }

        // available resolutions & rates
        if (availResos.isEmpty() || availRates.isEmpty() || availDepths.isEmpty())
        {
            availResos.clear();
            availRates.clear();
            availDepths.clear();

            ::ZeroMemory(&devMode, sizeof(devMode));
            devMode.dmSize = sizeof(devMode);

            DWORD i = 0;
            while (::EnumDisplaySettings(displayDevice.DeviceName, i++, &devMode))
            {
                const QSize reso(devMode.dmPelsWidth, devMode.dmPelsHeight);
                if (!availResos.contains(reso))
                    availResos += reso;
                if (!availRates.contains(reso, devMode.dmDisplayFrequency))
                    availRates.insertMulti(reso, devMode.dmDisplayFrequency);
                if (!availDepths.contains(reso, devMode.dmBitsPerPel))
                    availDepths.insertMulti(reso, devMode.dmBitsPerPel);

                ::ZeroMemory(&devMode, sizeof(devMode));
                devMode.dmSize = sizeof(devMode);
            }
        }
    }
}

bool QxtScreenPrivate::set(const QSize& reso, int rate, int depth)
{
    bool result = false;

    DISPLAY_DEVICE displayDevice;
    ::ZeroMemory(&displayDevice, sizeof(displayDevice));
    displayDevice.cb = sizeof(displayDevice);

    if (::EnumDisplayDevices(NULL, screen, &displayDevice, 0))
    {
        DEVMODE devMode;
        ::ZeroMemory(&devMode, sizeof(devMode));
        devMode.dmSize = sizeof(devMode);

        if (reso.isValid())
        {
            devMode.dmPelsWidth = reso.width();
            devMode.dmPelsHeight = reso.height();
            devMode.dmFields |= DM_PELSWIDTH | DM_PELSHEIGHT;
        }

        if (rate != -1)
        {
            devMode.dmDisplayFrequency = rate;
            devMode.dmFields |= DM_DISPLAYFREQUENCY;
        }

        if (depth != -1)
        {
            devMode.dmBitsPerPel = depth;
            devMode.dmFields |= DM_BITSPERPEL;
        }

        result = ::ChangeDisplaySettingsEx(displayDevice.DeviceName, &devMode, NULL, 0, NULL) == DISP_CHANGE_SUCCESSFUL;
    }

    return result;
}
