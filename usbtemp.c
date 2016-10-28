#include <stdio.h>
#include "k_ds18b20.h"

struct applet_arg {
  void *u;
  int argc;
  char **argv;
};

struct applet {
  char *name;
  int (*fn)(struct applet_arg *);
  char *usage;
  char *explaination;
};

static int get_number_of_sensors(struct s_usb_device *device) {

  int sensors;

  sensors = kds_get_number_of_sensors(device);
  if ( sensors < 0 ) {
    fprintf(stderr, "Cannot obtain number of sensors!\n");
    return -1;
  }
  else if ( sensors == 0 ) {
    fprintf(stderr, "No sensors attached!\n");
    return -1;
  }

  return sensors;
}

int applet_kds_config(struct applet_arg *arg) {

  int rv, cfg, cur_sensor, sensors;

  if ( arg->argc != 4 ) {
    fprintf(stderr, "Missing argument!\n");
    return 1;
  }

  if ( (sensors = get_number_of_sensors((struct s_usb_device *)arg->u)) < 0 )
    return 1;

  cur_sensor = atoi(arg->argv[2]);
  if ( cur_sensor >= sensors )
    return 1;

  cfg = atoi(arg->argv[3]);

  rv = kds_configure((struct s_usb_device *)arg->u, cur_sensor, cfg);
  if ( rv == 0 ) {
    printf("OK\n");
    return 0;
  }
  if ( rv == KDS_ERANGE )
    fprintf(stderr, "Invalid argument!\n");
  else 
    fprintf(stderr, "Error %d!\n", rv);

  return 1;
}

/* Detect new sensors. */
int applet_kds_detect(struct applet_arg *arg) {

  if ( kds_detect_sensors((struct s_usb_device *)arg->u) == 0 ) {
    printf("OK\n");
    return 0;
  }

  return 1;
}

int applet_kds_measure(struct applet_arg *arg) {

  int rv;

  rv = kds_measure((struct s_usb_device *)arg->u);

  if ( rv == 0 ) {
    printf("OK\n");
    return 0;
  }
  if ( rv == KDS_DEVBUSY )
    fprintf(stderr, "Device busy!\n");
  else 
    fprintf(stderr, "Error %d!\n", rv);

  return 1;
}

int applet_kds_modeget(struct applet_arg *arg) {

  struct s_usb_device *device;
  int r_bytes;
  char r_buffer[16];

  device = (struct s_usb_device *)arg->u;
  r_bytes = s_usb_send_request(device, USB_GETMODE, 0, (char*)&r_buffer, sizeof(r_buffer));

  if ( r_bytes == 1 ) {
    printf("Mode: %d\n", (int)r_buffer[0]);
    return 0;
  }

  return 1;
}

int applet_kds_modeset(struct applet_arg *arg) {

  struct s_usb_device *device;
  int r_bytes;
  char r_buffer[16];
  int cfg;

  if ( arg->argc < 3 ) {
    fprintf(stderr, "Missing argument!\n");
    return 1;
  }

  cfg = atoi(arg->argv[2]);
  if ( cfg < 0 || cfg > 1 ) {
    fprintf(stderr, "Invalid argument!\n");
    return 1;
  }

  device = (struct s_usb_device *)arg->u;  
  r_bytes = s_usb_send_request(device, USB_SETMODE, cfg, (char*)&r_buffer, sizeof(r_buffer));
  if ( r_bytes < 1 )
    return 1;
  
  printf("Mode set: %d\n", (int)r_buffer[0]);

  return 0;
}

int applet_kds_readrom(struct applet_arg *arg) {

  struct s_usb_device *device;
  char rom[OW_ROMCODE_SIZE];
  int i, cur_sensor, sensors;

  device = (struct s_usb_device *)arg->u;

  if ( (sensors = get_number_of_sensors(device)) < 0 )
    return 1;

  if ( arg->argc == 3 ) {
    cur_sensor = atoi(arg->argv[2]);
    if ( cur_sensor < sensors )
      sensors = cur_sensor + 1;
    else
      return 1;
  }

  for ( cur_sensor = 0; cur_sensor < sensors; cur_sensor++ ) {
    if ( kds_read_rom(device, cur_sensor, rom) == 0 ) {
      printf("S:%d R: ", cur_sensor);
      for ( i = 0; i < OW_ROMCODE_SIZE; i++ ) {
        printf("%02x", (unsigned char)rom[i]);
      }
      printf("\n");
    }
  }

  return 0;
}

int applet_kds_readtemp(struct applet_arg *arg) {

  struct s_usb_device *device;
  int T, cur_sensor, sensors;

  device = (struct s_usb_device *)arg->u;

  if ( (sensors = get_number_of_sensors(device)) < 0 )
    return 1;

  if ( arg->argc == 3 ) {
    cur_sensor = atoi(arg->argv[2]);
    if ( cur_sensor < sensors )
      sensors = cur_sensor + 1;
    else
      return 1;
  }

  for ( cur_sensor = 0; cur_sensor < sensors; cur_sensor++ ) {
    if ( kds_read_temperature(device, cur_sensor, &T) == 0 )
      printf("S:%d T: %.2f °C\n", cur_sensor, (float)(T)/16);
    else 
      fprintf(stderr, "S:%d not present.\n", cur_sensor);
  }

  return 0;
}

struct applet applets[] = {
  { "config", applet_kds_config, "[sensor] [0-3]", "Set precision of measurement representation of chosen sensor: 0 = 9 bit ... 3 = 12 bit." },
  { "detect", applet_kds_detect, "", "Detect DS18X20 sensors on 1W bus" },
  { "getmode", applet_kds_modeget, "", NULL },
  { "measure", applet_kds_measure, "", NULL },
  { "rom", applet_kds_readrom, "[?sensor]", "Display ROM (sensor serial number) from chosen sensor (if passed [sensor]) or from all." },
  { "setmode", applet_kds_modeset, "[mode: 0|1]", NULL },
  { "temp", applet_kds_readtemp, "[?sensor]", "Display temperature from chosen sensor (if passed [sensor]) or from all." },
  { NULL, NULL, NULL, NULL }
};

static int str_match(const char *a, const char *b) {

  int i;

  i = 0;
  while ( *(a + i) && *(b + i) && *(a + i) == *(b + i) )
    i++;

  return i;
}

static int find_applet(int argc, char **argv, struct applet **sel_applet) {

  struct applet *cur_command;
  int cur_match, sel_match;

  if ( argc < 2 )
    return -1;

  sel_match = 0;
  *sel_applet = NULL;

  for ( cur_command = applets; cur_command->name; cur_command++ ) {

    if ( (cur_match = str_match(cur_command->name, argv[1])) == 0 )
      continue;

    if ( cur_match > sel_match ) {
      sel_match = cur_match;
      *sel_applet = cur_command;
    }
    else if ( cur_match == sel_match )
      return -1;

  }

  return sel_match;
}

static void print_usage(char* name) {

  struct applet *cur_applet;

  fprintf(stdout, "Usage:\n");

  for ( cur_applet = applets; cur_applet->name; cur_applet++ ) {
    fprintf(stdout, "%s %s %s\n", name, cur_applet->name, cur_applet->usage);
    if (cur_applet->explaination != NULL )
      fprintf(stdout, "\t%s\n\n", cur_applet->explaination);
  }
}

int main(int argc, char **argv) {

  struct s_usb_device *device;
  struct applet *applet;
  struct applet_arg arg;
  int rv;

  printf("USBtemp CLI 2016\n\n");

  applet = NULL;
  rv = find_applet(argc, argv, &applet);
  if ( rv < 0 ) {
    print_usage(argv[0]);
    return 1;
  }
  else if ( rv == 0 ) {
    fprintf(stdout, "Applet not found!\n");
    return 1;
  }

  /* Obtain USB device. */
  device = NULL;
  if ( kds_get_device(&device) == KDS_NODEVICE ) {
    fprintf(stderr, "Could not connect to USB device!\n");
    return 1;
  }

  arg.argc = argc;
  arg.argv = argv;
  arg.u = (void *)device;

  /* Run applet. */
  if ( applet != NULL ) {
    rv = applet->fn(&arg);
  }

  kds_destroy_device(device);
  return rv;
}
