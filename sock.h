#ifndef NETWORK_SOCK_H
#define NETWORK_SOCK_H

u_int16_t checksum(u_int_8_t *data, int len);
u_int16_t checksum2(u_int8_t *data1, int len1, u_int8_t *data2, int len2);

#endif //NETWORK_SOCK_H
