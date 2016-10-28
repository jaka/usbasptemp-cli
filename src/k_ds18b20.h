#ifndef K_DS18B20_H
#define K_DS18B20_H

#include "s_usb.h"
#include "s_crc.h"

#define KDS_ERROR -1
#define KDS_ENOSENSORS -2	/* No sensors connected */
#define KDS_EUSB -3
#define KDS_NODEVICE -4
#define KDS_EBADCRC -5
#define KDS_DEVBUSY -6
#define KDS_ERANGE -7

#define KDS_VENDOR_NAME "KEL"
#define KDS_VENDOR_ID 0x03eb
#define KDS_PRODUCT_NAME "USBtemp"
#define KDS_PRODUCT_ID 0x0003

#define USB_MEASURE		1
#define USB_READ_TEMP	2
#define USB_READ_NUMBER	3
#define USB_READ_ROM	4
#define USB_DETECT		5
#define USB_CONFIG		6
#define USB_SETMODE		7
#define USB_GETMODE		8

#define OW_ROMCODE_SIZE 8
#define DS18X20_SP_SIZE 9
#define DS18X20_GENERATOR	0x8c

int kds_configure(struct s_usb_device *, int, int);
int kds_detect_sensors(struct s_usb_device *);
int kds_destroy_device(struct s_usb_device *);
int kds_get_device(struct s_usb_device **);
int kds_get_number_of_sensors(struct s_usb_device *);
int kds_measure(struct s_usb_device *);
int kds_read_temperature(struct s_usb_device *, int, int *);
int kds_read_rom(struct s_usb_device *, int, char *);

#endif
