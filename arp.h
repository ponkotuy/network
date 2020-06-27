
#ifndef NETWORK_ARP_H
#define NETWORK_ARP_H

char *my_arp_ip_ntoa_r(u_int8_t ip[4], char *buf);
int ArpAddTable(u_int8_t mac[6], struct in_addr *ipaddr);
int ArpRecv(int soc, struct ether_header *eh, u_int8_t *data, int len);

#endif //NETWORK_ARP_H
