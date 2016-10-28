#include "s_usb.h"

const char *s_usb_errors_msgs[] = {
  "Warning: cannot open USB device",
  "Warning: cannot query manufacturer for device",
  "Warning: cannot query product for device",
};

unsigned int s_usb_error;

/* Functions */

static int s_usbGetDescriptorString(usb_dev_handle *dev, int index, int langid, char *string, int string_len) {

  char r_buffer[256];
  int r_bytes, i;

  /* Make standard request GET_DESCRIPTOR with type string and given index. */
  r_bytes = usb_control_msg(dev, 
      USB_TYPE_STANDARD | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
      USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + index, langid, 
      r_buffer, sizeof(r_buffer), 1000);

  if (r_bytes < 0)
    return r_bytes;

  /* First byte is the data length. */
  if ((unsigned char)r_buffer[0] < r_bytes)
    r_bytes = (unsigned char)r_buffer[0];

  /* Second byte is the data type. */
  if (r_buffer[1] != USB_DT_STRING)
    return 0;

  /* Since the content of buffer is UTF-16LE encoded, actual number of chars is only half of r_bytes. */
  r_bytes /= 2;

  /* Convert to ISO Latin1. */
  for (i = 1; i < r_bytes && i < string_len; i++) {
    if (r_buffer[2 * i + 1] == 0)
      string[i - 1] = r_buffer[2 * i];
    else
      string[i - 1] = '?';
  }

  string[i - 1] = 0;
  return i - 1;
}

static int s_usbFindDevice(struct s_usb_device *device) {

  struct usb_bus *bus;
  struct usb_device *dev;
  char this_vendorName[256], this_productName[256];

  usb_init();
  usb_find_busses();
  usb_find_devices();

  for (bus = usb_get_busses(); bus; bus = bus->next) {
    for (dev = bus->devices; dev; dev = dev->next) {

      if ( dev->descriptor.idVendor != device->vendorId || dev->descriptor.idProduct != device->productId )
        continue;

      /* Open USB device. */
      if ( !(device->handle = usb_open(dev)) ) {
        s_usb_error = S_USB_ECANTOPEN;
        continue;
      }

      /* Get vendor name. */
      if ( s_usbGetDescriptorString(device->handle, dev->descriptor.iManufacturer, 0x0409, this_vendorName, sizeof(this_vendorName)) < 0 ) {
        s_usb_error = S_USB_ECANTQRYVEN;
        usb_close(device->handle);
        continue;
      }

      /* Get product name. */
      if ( s_usbGetDescriptorString(device->handle, dev->descriptor.iProduct, 0x0409, this_productName, sizeof(this_productName)) < 0 ) {
        s_usb_error = S_USB_ECANTQRYPRD;
        usb_close(device->handle);
        continue;
      }

      /* Match names. */
      if ( !strcmp(this_vendorName, device->vendorName) && !strcmp(this_productName, device->productName) )
        return S_USB_SUCCESS;
      else
        usb_close(device->handle);
    }
  }
  return S_USB_FAILURE;
}

struct s_usb_device *s_usb_create_device(int vendorId, char *vendorName, int productId, char *productName) {

  struct s_usb_device *device = NULL;

  device = malloc(sizeof(struct s_usb_device));
  if ( device ) {
    /* Fill device struct. */
    device->vendorId = vendorId;
    device->productId = productId;
    device->vendorName = vendorName;
    device->productName = productName;
    device->handle = NULL;
    if ( s_usbFindDevice(device) < 0 ) {
      free(device);
      return NULL;
    }
  }
  return device;
}

void s_usb_destroy(struct s_usb_device *device) {

  if ( device ) {
    usb_close(device->handle);
    free(device);
  }
}

const char *s_usb_error_msg(void) {

  if ( 0 < s_usb_error && s_usb_error < S_USB_E_NR)
    return s_usb_errors_msgs[s_usb_error - 1];
  return NULL;
}

int s_usb_send_request(struct s_usb_device *device, int request, int data, char* r_buffer, int r_buffer_size) {

  return usb_control_msg(device->handle, 
      USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
      request, data, 0, r_buffer, r_buffer_size, S_USB_TIMEOUT);
}

int s_usb_send_data_short(struct s_usb_device *device, int request, int data, char* r_buffer, int r_buffer_size) {

  return usb_control_msg(device->handle, 
      USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
      request, data & 0xffff, (data >> 16) & 0xffff, r_buffer, r_buffer_size, S_USB_TIMEOUT);
}

int s_usb_send_data(struct s_usb_device *device, int request, int data, char* w_buffer) {

  return usb_control_msg(device->handle, 
      USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 
      request, data, 0, w_buffer, strlen(w_buffer) + 1, S_USB_TIMEOUT);
}
