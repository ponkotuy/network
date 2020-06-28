
#ifndef NETWORK_ARP_H
#define NETWORK_ARP_H

char *my_arp_ip_ntoa_r(u_int8_t ip[4], char *buf);
int ArpAddTable(u_int8_t mac[6], struct in_addr *ipaddr);
int ArcDelTable(struct in_addr *ipaddr);
int ArpSearchTable(struct in_addr *ipaddr, u_int8_t mac[6]);
int ArpShowTable();
int ArpSend(int soc, u_int16_t op,
            u_int8_t e_smac[6], u_int8_t e_dmac[6],
            u_int8_t smac[6], u_int8_t dmac[6],
            u_int8_t saddr[4], u_int8_t daddr[4]);
int ArpRecv(int soc, struct ether_header *eh, u_int8_t *data, int len);

#endif //NETWORK_ARP_H
