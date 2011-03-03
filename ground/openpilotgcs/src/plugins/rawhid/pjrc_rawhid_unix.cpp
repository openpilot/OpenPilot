/* @file pjrc_rawhid_unix.cpp
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup RawHIDPlugin Raw HID Plugin
 * @{
 * @brief Impliments a HID USB connection to the flight hardware as a QIODevice
 *****************************************************************************/

/* Simple Raw HID functions for Linux - for use with Teensy RawHID example
 * http://www.pjrc.com/teensy/rawhid.html
 * Copyright (c) 2009 PJRC.COM, LLC
 *
 *  rawhid_open - open 1 or more devices
 *  rawhid_recv - receive a packet
 *  rawhid_send - send a packet
 *  rawhid_close - close a device
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above description, website URL and copyright notice and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Version 1.0: Initial Release
 */

#include "pjrc_rawhid.h"

#define printf qDebug

pjrc_rawhid::pjrc_rawhid()
{
    first_hid = NULL;
    last_hid = NULL;
}

pjrc_rawhid::~pjrc_rawhid()
{
}

//  open - open 1 or more devices
//
//    Inputs:
//	max = maximum number of devices to open
//	vid = Vendor ID, or -1 if any
//	pid = Product ID, or -1 if any
//	usage_page = top level usage page, or -1 if any
//	usage = top level usage number, or -1 if any
//    Output:
//	actual number of devices opened
//
int pjrc_rawhid::open(int max, int vid, int pid, int usage_page, int usage)
{
	struct usb_bus *bus;
	struct usb_device *dev;
	struct usb_interface *iface;
	struct usb_interface_descriptor *desc;
	struct usb_endpoint_descriptor *ep;
	usb_dev_handle *u;
	uint8_t buf[1024], *p;
	int i, n, len, tag, ep_in, ep_out, count=0, claimed;
	uint32_t val=0, parsed_usage, parsed_usage_page;
	hid_t *hid;

	if (first_hid) free_all_hid();
    //printf("pjrc_rawhid_open, max=%d\n", max);

	if (max < 1) return 0;

	usb_init();
	usb_find_busses();
	usb_find_devices();

	for (bus = usb_get_busses(); bus; bus = bus->next)
	{
		for (dev = bus->devices; dev; dev = dev->next)
		{
			if (vid > 0 && dev->descriptor.idVendor != vid) continue;
			if (pid > 0 && dev->descriptor.idProduct != pid) continue;
			if (!dev->config) continue;
			if (dev->config->bNumInterfaces < 1) continue;
			printf("device: vid=%04X, pic=%04X, with %d iface",
                   dev->descriptor.idVendor,
                   dev->descriptor.idProduct,
                   dev->config->bNumInterfaces);
			iface = dev->config->interface;
			u = NULL;
			claimed = 0;
			for (i=0; i<dev->config->bNumInterfaces && iface; i++, iface++)
			{
				desc = iface->altsetting;
				if (!desc) continue;

				printf("  type %d, %d, %d", desc->bInterfaceClass, desc->bInterfaceSubClass, desc->bInterfaceProtocol);

				if (desc->bInterfaceClass != 3) continue;
				if (desc->bInterfaceSubClass != 0) continue;
				if (desc->bInterfaceProtocol != 0) continue;

				ep = desc->endpoint;
				ep_in = ep_out = 0;
				for (n = 0; n < desc->bNumEndpoints; n++, ep++)
				{
					if (ep->bEndpointAddress & 0x80)
					{
						if (!ep_in) ep_in = ep->bEndpointAddress & 0x7F;
						qDebug() <<  "    IN endpoint " << ep_in;
					}
					else
					{
						if (!ep_out) ep_out = ep->bEndpointAddress;
						qDebug() << "    OUT endpoint " <<  ep_out;
					}
				}
				if (!ep_in) continue;

				if (!u)
				{
					u = usb_open(dev);
					if (!u)
					{
						qDebug() << "  unable to open device";
						break;
					}
				}
				qDebug() << "  hid interface (generic)";
				if (usb_get_driver_np(u, i, (char *)buf, sizeof(buf)) >= 0)
				{
					printf("  in use by driver \"%s\"", buf);
					if (usb_detach_kernel_driver_np(u, i) < 0)
					{
						printf("  unable to detach from kernel");
						continue;
					}
				}

				if (usb_claim_interface(u, i) < 0)
				{
					printf("  unable claim interface %d", i);
					continue;
				}

				len = usb_control_msg(u, 0x81, 6, 0x2200, i, (char *)buf, sizeof(buf), 250);
				printf("  descriptor, len=%d", len);
				if (len < 2)
				{
					usb_release_interface(u, i);
					continue;
				}

				p = buf;
				parsed_usage_page = parsed_usage = 0;
				while ((tag = hid_parse_item(&val, &p, buf + len)) >= 0)
				{
					printf("  tag: %X, val %X", tag, val);
					if (tag == 4) parsed_usage_page = val;
					if (tag == 8) parsed_usage = val;
					if (parsed_usage_page && parsed_usage) break;
				}
				if ((!parsed_usage_page) || (!parsed_usage) ||
                    (usage_page > 0 && parsed_usage_page != (uint32_t)usage_page) ||
					(usage > 0 && parsed_usage != (uint32_t)usage))
				{
					usb_release_interface(u, i);
					continue;
				}

				hid = (struct hid_struct *)malloc(sizeof(struct hid_struct));
				if (!hid)
				{
					usb_release_interface(u, i);
					continue;
				}

				hid->usb = u;
				hid->iface = i;
				hid->ep_in = ep_in;
				hid->ep_out = ep_out;
				hid->open = 1;
				add_hid(hid);

				claimed++;
				count++;
				if (count >= max) return count;
			}

			if (u && !claimed) usb_close(u);
		}
	}

	return count;
}

//  recveive - receive a packet
//    Inputs:
//	num = device to receive from (zero based)
//	buf = buffer to receive packet
//	len = buffer's size
//	timeout = time to wait, in milliseconds
//    Output:
//	number of bytes received, or -1 on error
//
int pjrc_rawhid::receive(int num, void *buf, int len, int timeout)
{
	if (!buf) return -1;

	hid_t *hid = get_hid(num);
    if (!hid || !hid->open) return -1;

	int r = usb_interrupt_read(hid->usb, hid->ep_in, (char *)buf, len, timeout);
    if (r >= 0) return r;
    if (r == -110) return 0;  // timeout

    return -1;
}

//  send - send a packet
//    Inputs:
//	num = device to transmit to (zero based)
//	buf = buffer containing packet to send
//	len = number of bytes to transmit
//	timeout = time to wait, in milliseconds
//    Output:
//	number of bytes sent, or -1 on error
//
int pjrc_rawhid::send(int num, void *buf, int len, int timeout)
{
    hid_t *hid;

    hid = get_hid(num);
    if (!hid || !hid->open) return -1;
    if (hid->ep_out) {
        return usb_interrupt_write(hid->usb, hid->ep_out, (char *)buf, len, timeout);
    } else {
        return usb_control_msg(hid->usb, 0x21, 9, 0, hid->iface, (char *)buf, len, timeout);
    }
}

//  getserial - get the serialnumber of the device
//
//    Inputs:
//	num = device to close (zero based)
//      buf = buffer to read the serialnumber into
//    Output
//	number of bytes in found, or -1 on error
//
QString pjrc_rawhid::getserial(int num) {
    hid_t *hid;
    char buf[128];
    hid = get_hid(num);
    if (!hid || !hid->open) return QString("");

    int retlen = usb_get_string_simple(hid->usb, 3, buf, 128);
    return QString().fromAscii(buf,-1);
}

//  close - close a device
//
//    Inputs:
//	num = device to close (zero based)
//    Output
//	(nothing)
//
void pjrc_rawhid::close(int num)
{
	hid_close(get_hid(num));
}

// Chuck Robey wrote a real HID report parser
// (chuckr@telenix.org) chuckr@chuckr.org
// http://people.freebsd.org/~chuckr/code/python/uhidParser-0.2.tbz
// this tiny thing only needs to extract the top-level usage page
// and usage, and even then is may not be truly correct, but it does
// work with the Teensy Raw HID example.
int pjrc_rawhid::hid_parse_item(uint32_t *val, uint8_t **data, const uint8_t *end)
{
    const uint8_t *p = *data;
    uint8_t tag;
    int table[4] = {0, 1, 2, 4};
    int len;

    if (p >= end) return -1;
    if (p[0] == 0xFE) {
        // long item, HID 1.11, 6.2.2.3, page 27
        if (p + 5 >= end || p + p[1] >= end) return -1;
        tag = p[2];
        *val = 0;
        len = p[1] + 5;
    } else {
        // short item, HID 1.11, 6.2.2.2, page 26
        tag = p[0] & 0xFC;
        len = table[p[0] & 0x03];
        if (p + len + 1 >= end) return -1;
        switch (p[0] & 0x03) {
        case 3: *val = p[1] | (p[2] << 8) | (p[3] << 16) | (p[4] << 24); break;
        case 2: *val = p[1] | (p[2] << 8); break;
        case 1: *val = p[1]; break;
        case 0: *val = 0; break;
        }
    }
    *data += len + 1;
    return tag;
}

void pjrc_rawhid::add_hid(hid_t *h)
{
	if (!h) return;

	if (!first_hid || !last_hid)
	{
        first_hid = last_hid = h;
        h->next = h->prev = NULL;
        return;
    }
    last_hid->next = h;
    h->prev = last_hid;
    h->next = NULL;
    last_hid = h;
}

hid_t * pjrc_rawhid::get_hid(int num)
{
	hid_t *p = NULL;
    for (p = first_hid; p && num > 0; p = p->next, num--) ;
    return p;
}

void pjrc_rawhid::free_all_hid(void)
{
	for (hid_t *p = first_hid; p; p = p->next)
        hid_close(p);

	hid_t *p = first_hid;
	while (p)
	{
		hid_t *q = p;
        p = p->next;
        free(q);
    }

    first_hid = last_hid = NULL;
}

void pjrc_rawhid::hid_close(hid_t *hid)
{
	if (!hid) return;
	if (!hid->open) return;

        usb_release_interface(hid->usb, hid->iface);

	int others = 0;
	for (hid_t *p = first_hid; p; p = p->next)
	{
		if (p->open && p->usb == hid->usb)
			others++;
    }
	if (!others)
		usb_close(hid->usb);

    hid->usb = NULL;
    hid->open = 0;
}
