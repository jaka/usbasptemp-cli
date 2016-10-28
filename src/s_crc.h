#ifndef S_CRC_H
#define S_CRC_H

char msb_crc8(char* data_in, unsigned int len, const unsigned char generator);
char lsb_crc8(char* data_in, unsigned int len, const unsigned char generator);

#endif
