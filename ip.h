
#ifndef NETWORK_IP_H
#define NETWORK_IP_H

int IpRecv(int soc, u_int8_t *raw, int raw_len, struct ether_header *eh, u_int8_t *data, int len);

#endif //NETWORK_IP_H
