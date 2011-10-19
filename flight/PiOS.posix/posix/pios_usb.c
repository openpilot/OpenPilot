#include <usb.h>
#include <libudev.h>
#include <stdio.h>
#include <string.h>

//  recveive - receive a packet
//    Inputs:
//	buf = buffer containing packet to send
//	len = number of bytes to transmit
//	timeout = time to wait, in milliseconds
//    Output:
//	number of bytes received, or -1 on error
//
int opreceive(usb_dev_handle *device, int endpoint, void *buf, int len, int timeout)
{
	if (!buf) return -1;
    if (!device) return -1;
    if (!endpoint) return -1;

	return usb_interrupt_read(device, endpoint, (char *)buf, len, timeout);
}

//  send - send a packet
//    Inputs:
//	buf = buffer containing packet to send
//	len = number of bytes to transmit
//	timeout = time to wait, in milliseconds
//    Output:
//	number of bytes sent, or -1 on error
//
int opsend(usb_dev_handle *device, int endpoint, void *buf, int len, int timeout)
{
	if (!buf) return -1;
    if (!device) return -1;
    if (!endpoint) return -1;

	return usb_interrupt_write(device, endpoint, (char *)buf, len, timeout);
}




int main() {


	int vendor = 0x20a0;
	int product = 0x415a;


	struct usb_bus *bus;
	struct usb_device *dev;
	usb_dev_handle *dev_handle, *OpenPilot=NULL;
	struct usb_interface *iface;
	struct usb_interface_descriptor *desc;
	struct usb_endpoint_descriptor *ep;
	int ep_in,ep_out;
	unsigned char buf[1024];

	int claimed;

	usb_init();
	usb_find_busses();
	usb_find_devices();

	for (bus = usb_get_busses(); bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if (dev->descriptor.idVendor != vendor) continue;
			if (dev->descriptor.idProduct != product) continue;
			if (!dev->config) continue;
			if (dev->config->bNumInterfaces < 1) continue;
			printf("device: vid=%04X, pic=%04X, with %d iface",
                   dev->descriptor.idVendor,
                   dev->descriptor.idProduct,
                   dev->config->bNumInterfaces);
			iface = dev->config->interface;
			dev_handle = NULL;
			claimed = 0;
			int i;
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
				int n;
				for (n = 0; n < desc->bNumEndpoints; n++, ep++)
				{
					if (ep->bEndpointAddress & 0x80)
					{
						if (!ep_in) ep_in = ep->bEndpointAddress & 0x7F;
						printf("    IN endpoint %X\n",ep_in);
					}
					else
					{
						if (!ep_out) ep_out = ep->bEndpointAddress;
						printf("    OUT endpoint %X\n",ep_out);
					}
				}
				if (!ep_in) continue;

				if (!dev_handle)
				{
					dev_handle = usb_open(dev);
					if (!dev_handle)
					{
						printf("  unable to open device\n");
						break;
					}
				}
				printf("  hid interface (generic)\n");
				if (usb_get_driver_np(dev_handle, i, (char *)buf, sizeof(buf)) >= 0)
				{
					printf("  in use by driver \"%s\"", buf);
					if (usb_detach_kernel_driver_np(dev_handle, i) < 0)
					{
						printf("  unable to detach from kernel");
						continue;
					}
				}

				if (usb_claim_interface(dev_handle, i) < 0)
				{
					printf("  unable claim interface %d", i);
					continue;
				}

				int len;
				len = usb_control_msg(dev_handle, 0x81, 6, 0x2200, i, (char *)buf, sizeof(buf), 250);
				printf("  descriptor, len=%d\n", len);
				if (len < 2)
				{
					usb_release_interface(dev_handle, i);
					continue;
				}

				/*hid = (struct hid_struct *)malloc(sizeof(struct hid_struct));
				if (!hid)
				{
					usb_release_interface(dev_handle, i);
					continue;
				}*/

				printf("found :)))) \n");
				/*
				hid->usb = dev_handle;
				hid->iface = i;
				hid->ep_in = ep_in;
				hid->ep_out = ep_out;
				hid->open = 1;
				add_hid(hid);*/

				claimed++;
				//count++;
				//if (count >= max) return count;
				OpenPilot = dev_handle;
			}

			if (dev_handle && !claimed) usb_close(dev_handle);

			
		}
	}
if (!OpenPilot) return 0;

	char* buffer[1024];
	while (1) {
		int n = opreceive(OpenPilot,ep_in,buffer,1024,100);
		if (n>0) {
			fwrite( buffer, 1,n,stdout);
			opsend(OpenPilot,ep_out,buffer,n,100);
		} else printf("read error\n");
	}

}




