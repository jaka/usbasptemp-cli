#include "s_crc.h"

char msb_crc8(char* data_in, unsigned int len, const unsigned char generator) {

  unsigned int i;
  unsigned int bit_counter;
  char crc = 0;

  for ( i = 0; i < len; i++ ) {
    crc ^= *(data_in + i);
    bit_counter = 8;
    do {
      if ( crc & 0x80 )
        crc = ((crc << 1) ^ generator);
      else
        crc <<= 1;
      bit_counter--;
    } while ( bit_counter > 0 );
  }
  return crc;
}

char lsb_crc8(char *data_in, unsigned int len, const unsigned char generator) {

  unsigned int i;
  unsigned int bit_counter;
  char crc = 0;

  for ( i = 0; i < len; i++ ) {
    crc ^= *(data_in + i);
    bit_counter = 8;
    do {
      if ( crc & 0x01 )
        crc = (((crc >> 1) & 0x7f) ^ generator);
      else
        crc = (crc >> 1) & 0x7f;
      bit_counter--;
    } while ( bit_counter > 0 );
  }
  return crc;
}
