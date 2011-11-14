/**
 * This is a small test program that connects to OpenPilot/CC via USB and writes a .opl compatible stream to stdout
 */

#include <usb.h>
//#include <libudev.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>

//  recveive - receive a packet
//    Inputs:
//	buf = buffer containing packet to send
//	len = number of bytes to transmit
//	timeout = time to wait, in milliseconds
//    Output:
//	number of bytes received, or -1 on error
//
int opreceive(usb_dev_handle *device, int endpoint, void *buf, int timeout)
{
	int8_t tmpBuffer[64];
	if (!buf) return -1;
    if (!device) return -1;
    if (!endpoint) return -1;
	int received=0;


	received=usb_interrupt_read(device, endpoint, tmpBuffer, 64, timeout);
	if (received>0) {
		received=tmpBuffer[1];
		if (received>62) received=62;
		memcpy(buf,&tmpBuffer[2],received);
		return received;
	}
	return received;
}

int64_t timeDifference(struct timeval * old, struct timeval * new) {
	time_t seconds;
	int64_t mseconds;

	mseconds = (new->tv_usec - old->tv_usec)/1000;
	mseconds = mseconds + ( new->tv_sec - old->tv_sec ) *1000;

	return mseconds;

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

	usb_set_debug(99);
	usb_init();
	usb_find_busses();
	usb_find_devices();

	for (bus = usb_get_busses(); bus; bus = bus->next) {
		fprintf(stderr,"usbbus found\n");
		for (dev = bus->devices; dev; dev = dev->next) {
			fprintf(stderr,"usbdevice found - %04X:%04X\n",dev->descriptor.idVendor,dev->descriptor.idProduct);
			if (dev->descriptor.idVendor != vendor) continue;
			if (dev->descriptor.idProduct != product) continue;
			if (!dev->config) continue;
			if (dev->config->bNumInterfaces < 1) continue;
			fprintf(stderr,"device: vid=%04X, pic=%04X, with %d iface",
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

				fprintf(stderr,"  type %d, %d, %d", desc->bInterfaceClass, desc->bInterfaceSubClass, desc->bInterfaceProtocol);

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
						fprintf(stderr,"    IN endpoint %X\n",ep_in);
					}
					else
					{
						if (!ep_out) ep_out = ep->bEndpointAddress;
						fprintf(stderr,"    OUT endpoint %X\n",ep_out);
					}
				}
				if (!ep_in) continue;

				if (!dev_handle)
				{
					dev_handle = usb_open(dev);
					if (!dev_handle)
					{
						fprintf(stderr,"  unable to open device\n");
						break;
					}
				}
				fprintf(stderr,"  hid interface (generic)\n");
				if (usb_get_driver_np(dev_handle, i, (char *)buf, sizeof(buf)) >= 0)
				{
					fprintf(stderr,"  in use by driver \"%s\"", buf);
					if (usb_detach_kernel_driver_np(dev_handle, i) < 0)
					{
						fprintf(stderr,"  unable to detach from kernel");
						continue;
					}
				}

				if (usb_claim_interface(dev_handle, i) < 0)
				{
					fprintf(stderr,"  unable claim interface %d", i);
					continue;
				}

				int len;
				len = usb_control_msg(dev_handle, 0x81, 6, 0x2200, i, (char *)buf, sizeof(buf), 250);
				fprintf(stderr,"  descriptor, len=%d\n", len);
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

				fprintf(stderr,"found :)))) \n");
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
if (!OpenPilot) {
	fprintf(stderr,"no such device\n");
	return -1;
}

	char buffer[64];
	struct timeval starttime,ctime;
	gettimeofday(&starttime,NULL);
	int n = opreceive(OpenPilot,ep_in,buffer,0);
	while (n>0) {
		gettimeofday(&ctime,NULL);
		uint32_t timestamp = timeDifference(&starttime,&ctime);
		uint64_t dataSize  = n;

		fwrite( (uint8_t*) &timestamp,sizeof(timestamp),1,stdout);
		fwrite( (uint8_t*) &dataSize,sizeof(dataSize),1,stdout);
		fwrite( buffer, 1,dataSize,stdout);
		fprintf(stderr," %i: %i\n",timestamp,(uint32_t)dataSize);
		n = opreceive(OpenPilot,ep_in,buffer,0);
	}

}




