#ifndef S_USB_H
#define S_USB_H

#include <string.h>
#include <usb.h>

#define S_USB_TIMEOUT 5000

/* Response values */
#define S_USB_SUCCESS 0
#define S_USB_FAILURE -1

/* Error messages */
#define S_USB_ECANTOPEN 1
#define S_USB_ECANTQRYVEN 2
#define S_USB_ECANTQRYPRD 3

#define S_USB_E_NR 4

extern unsigned int s_usb_error;

struct s_usb_device {
  usb_dev_handle *handle;
  int vendorId;
  int productId;
  char *vendorName;
  char *productName;
};

struct s_usb_device *s_usb_create_device(int vendorId, char *vendorName, int productId, char *productName);
void s_usb_destroy(struct s_usb_device *device);
const char *s_usb_error_msg(void);

int s_usb_send_request(struct s_usb_device *device, int request, int data, char* r_buffer, int r_buffer_size);
int s_usb_send_data_short(struct s_usb_device *device, int request, int data, char* r_buffer, int r_buffer_size);
int s_usb_send_data(struct s_usb_device *device, int request, int data, char* w_buffer);

#endif
