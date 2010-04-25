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

#ifdef HAVE_XRANDR
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#endif // HAVE_XRANDR

void QxtScreenPrivate::init_sys()
{
#ifdef HAVE_XRANDR
    Display* display = XOpenDisplay(NULL);
    Window root = RootWindow(display, screen);
    XRRScreenConfiguration* config = XRRGetScreenInfo(display, root);

    // available resolutions & rates
    if (availResos.isEmpty() || availRates.isEmpty())
    {
        availResos.clear();
        availRates.clear();

        int sizeCount = 0;
        XRRScreenSize* sizes = XRRSizes(display, 0, &sizeCount);
        for (int i = 0; i < sizeCount; ++i)
        {
            QSize reso(sizes[i].width, sizes[i].height);
            if (!availResos.contains(reso))
                availResos += reso;

            int rateCount = 0;
            short* rates = XRRConfigRates(config, i, &rateCount);
            for (int j = 0; j < rateCount; ++j)
                availRates.insertMulti(reso, rates[j]);
        }
    }

    // current resolution & rate
    if (!currReso.isValid() || currRate < 0)
    {
        Rotation rotation;
        SizeID sizeId = XRRConfigCurrentConfiguration(config, &rotation);
        currReso = availResos.at(sizeId);
        currRate = XRRConfigCurrentRate(config);
    }

    XRRFreeScreenConfigInfo(config);
    XCloseDisplay(display);
#endif // HAVE_XRANDR
}

bool QxtScreenPrivate::set(const QSize& reso, int rate, int depth)
{
    bool result = false;
    Q_UNUSED(reso);
    Q_UNUSED(rate);
    Q_UNUSED(depth);
#ifdef HAVE_XRANDR
    Display* display = XOpenDisplay(NULL);
    Window root = RootWindow(display, screen);
    XRRScreenConfiguration* config = XRRGetScreenInfo(display, root);

    int sizeIndex = 0;
    if (reso.isValid())
        sizeIndex = availResos.indexOf(reso);
    else
        sizeIndex = availResos.indexOf(currReso);
    Q_ASSERT(sizeIndex != -1);

    if (rate == -1)
        result = XRRSetScreenConfig(display, config, root, sizeIndex, RR_Rotate_0, CurrentTime);
    else
        result = XRRSetScreenConfigAndRate(display, config, root, sizeIndex, RR_Rotate_0, rate, CurrentTime);

    XRRFreeScreenConfigInfo(config);
    XCloseDisplay(display);
#endif // HAVE_XRANDR
    return result;
}
