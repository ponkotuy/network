
#include <stdio.h>
#include <string.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include "ip.h"
#include "sock.h"

#define IP_RECV_BUF_NO (16)

typedef struct {
    time_t timestamp;
    int id;
    u_int8_t data[64 * 1024];
    int len;
} IP_RECV_BUF;

IP_RECV_BUF IpRecvBuf[IP_RECV_BUF_NO];

int IpRecv(int soc, u_int8_t *raw, int raw_len, struct ether_header *eh, u_int8_t *data, int len) {
    struct ip *ip;
    u_int8_t option[1500];
    u_int16_t sum;
    int optionLen, no, off, plen;
    u_int8_t *ptr = data;

    if(len < (int) sizeof(struct ip)) {
        printf("len(%d) < sizeof(struct ip)\n", len);
        return -1;
    }
    ip = (struct ip *)ptr;
    ptr += sizeof(struct ip);
    len -= sizeof(struct ip);

    optionLen = ip->ip_hl * 4 - sizeof(struct ip);
    if(0 < optionLen) {
        if(1500 <= optionLen) {
            printf("IP optionLen(%d) too big\n", optionLen);
            return -1;
        }
        memcpy(option, ptr, optionLen);
        ptr += optionLen;
        len -= optionLen;
    }

    if(optionLen == 0) {
        sum = checksum((u_int8_t *)ip, sizeof(struct ip));
    } else {
        sum = checksum2((u_int8_t *)ip, sizeof(struct ip), option, optionLen);
    }
    if(sum != 0 && sum != 0xFFFF) {
        printf("bad ip checksum\n");
        return -1;
    }

    plen = ntohs(ip->ip_len) - ip->ip_hl;

    no = IpRecvBufAdd(ntohs(ip->ip_id));
    off = (ntohs(ip->ip_off) & IP_OFFMASK) * 8;
    memcpy(IpRecvBuf[no].data + off, ptr, len);
    if(!(ntohs(ip->ip_off) & IP_MF)) {
        IpRecvBuf[no].len = off + plen;
        if(ip->ip_p == IPPROTO_ICMP) {
            IcmpRecv(soc, raw, raw_len, eh, ip, IpRecvBuf[no].data, IpRecvBuf[no].len);
        }
        IpRecvBufDel(ntohs(ip->ip_id));
    }

    return 0;
}
