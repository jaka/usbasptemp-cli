#include "k_ds18b20.h"

int kds_configure(struct s_usb_device *device, int sensor, int cfg) {

  int r_bytes;
  char r_buffer[16];
  int req;

  if ( cfg < 0 || cfg > 3 )
    return KDS_ERANGE;

  req = (sensor | (cfg << 6)) & 0xff;

  r_bytes = s_usb_send_request(device, USB_CONFIG, req, (char*)&r_buffer, sizeof(r_buffer));
  if ( r_bytes < 1 )
    return KDS_ERROR;

  return 0;
}

int kds_configure_get(struct s_usb_device *device, int sensor, int *cfg) {

  int r_bytes;
  char r_buffer[16];

  r_bytes = s_usb_send_request(device, USB_PRECISION_GET, sensor, (char*)&r_buffer, sizeof(r_buffer));
  if (r_bytes != 1)
    return KDS_ERROR;
  *cfg = (int)r_buffer[0];
  return 0;
}

int kds_detect_sensors(struct s_usb_device *device) {

  int r_bytes;
  char r_buffer[16];

  r_bytes = s_usb_send_request(device, USB_DETECT, 0, (char*)&r_buffer, sizeof(r_buffer));
  if ( r_bytes == 1 && r_buffer[0] == 1 )
    return 0;

  return KDS_ERROR;
}

int kds_destroy_device(struct s_usb_device *device) {

  s_usb_destroy(device);
  return 0;
}

int kds_get_device(struct s_usb_device **device) {

  *device = s_usb_create_device(KDS_VENDOR_ID, KDS_VENDOR_NAME, KDS_PRODUCT_ID, KDS_PRODUCT_NAME);
  if ( *device == NULL )
    return KDS_NODEVICE;

  return 0;
}

int kds_get_number_of_sensors(struct s_usb_device *device) {

  int r_bytes;
  char r_buffer[16];

  r_bytes = s_usb_send_request(device, USB_READ_NUMBER, 0, (char*)&r_buffer, sizeof(r_buffer));
  return ( r_bytes < 1 ) ? KDS_ENOSENSORS : (int)r_buffer[0];
}

int kds_measure(struct s_usb_device *device) {

  int r_bytes;
  char r_buffer[16];

  r_bytes = s_usb_send_request(device, USB_MEASURE, 0, (char*)&r_buffer, sizeof(r_buffer));

  if ( r_bytes == 1 )
    return 0;
  else if ( r_bytes == 0 )
    return KDS_DEVBUSY;

  return KDS_ERROR;
}

int kds_read_temperature(struct s_usb_device *device, int sensor, int *temperature) {

  int r_bytes;
  char r_buffer[16];
  int T;

  r_bytes = s_usb_send_request(device, USB_READ_TEMP, sensor, (char*)&r_buffer, sizeof(r_buffer));
  if ( r_bytes < 0 )
    return KDS_EUSB;

  if ( r_bytes != DS18X20_SP_SIZE )
    return KDS_ERROR;

  /*
  for (T = 0; T < 9; T++)
    printf("C: 0x%02x\n", (char)r_buffer[T]);
  */

  /* Content of r_buffer[4] tells if sensor is connected */
  if ( (r_buffer[4] & 0x9f) != 0x1f )
    return KDS_ERROR;

  /* Check CRC */
  if ( r_buffer[DS18X20_SP_SIZE - 1] != lsb_crc8(&r_buffer[0], DS18X20_SP_SIZE - 1, DS18X20_GENERATOR) )
    return KDS_EBADCRC;

  /* Format to integers T[1].T[0]
  T[1] = (r_buffer[1] << 4) | ((r_buffer[0] & 0xf0) >> 4);
  T[0] = ((r_buffer[0] & 0x0f) * 25) >> 2;
  */

  T = (r_buffer[1] << 8) + (r_buffer[0] & 0xff);

  if ( (T >> 15) & 0x01 ) {
    T--;
    T ^= 0xffff;
    T *= -1;
  }
  *temperature = T;

  return 0;
}

int kds_read_rom(struct s_usb_device *device, int sensor, char *rom) {

  int r_bytes;
  char r_buffer[16];

  r_bytes = s_usb_send_request(device, USB_READ_ROM, sensor, (char*)&r_buffer, sizeof(r_buffer));
  if ( r_bytes < 0 )
    return KDS_EUSB;

  if ( r_bytes != OW_ROMCODE_SIZE )
    return KDS_ERROR;

  /* Check CRC */
  if ( r_buffer[OW_ROMCODE_SIZE - 1] != lsb_crc8(&r_buffer[0], OW_ROMCODE_SIZE - 1, DS18X20_GENERATOR) )
    return KDS_EBADCRC;

  for ( r_bytes = 0; r_bytes < OW_ROMCODE_SIZE; r_bytes++ ) {
    rom[r_bytes] = r_buffer[r_bytes];
  }

  return 0;
}
