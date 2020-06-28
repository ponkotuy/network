#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include "arp.h"

extern PARAM Param;

u_int8_t AllZeroMac[6] = {0, 0, 0, 0, 0, 0};
u_int8_t BcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

char *my_ether_ntoa_r(u_int8_t *hwaddr, char *buf) {
    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
    return buf;
}

void print_ether_header(struct ether_header *eh) {
    char buf1[80];
    printf("---ether-header---\n");
    printf("ether_dhost=%s\n", my_ether_ntoa_r(eh->ether_dhost, buf1));
    printf("ether_shost=%s\n", my_ether_ntoa_r(eh->ether_shost, buf1));
    printf("ether_type=%02X", ntohs(eh->ether_type));
    switch (ntohs(eh->ether_type)) {
        case ETHERTYPE_PUP:
            printf("(Xerox PUP)\n");
            break;
        case ETHERTYPE_IP:
            printf("(IP)\n");
            break;
        case ETHERTYPE_ARP:
            printf("(Address resolution)\n");
            break;
        case ETHERTYPE_REVARP:
            printf("(Reverse ARP)\n");
            break;
        default:
            printf("(unknown)\n");
            break;
    }
    return;
}

int EtherSend(int soc, u_int8_t smac[6], u_int8_t dmac[6], u_int16_t type, u_int8_t *data, int len) {
    struct ether_header *eh;
    u_int8_t *ptr, sbuf[sizeof(struct ether_header)+ETHERMTU];
    int padlen;

    if(ETHERMTU < len) {
        printf("EtherSend: data too long:%d\n", len);
        return -1;
    }

    ptr = sbuf;
    eh = (struct ether_header *)ptr;
    memset(eh, 0, sizeof(struct ether_header));
    memcpy(eh->ether_dhost, dmac, 6);
    memcpy(eh->ether_shost, smac, 6);
    eh->ether_type = htons(type);
    ptr += sizeof(struct ether_header);

    memcpy(ptr, data, len);
    ptr += len;

    // frame sizeがETH_ZLEN(60)より小さい場合はETH_ZLENまでパディングする
    if((ptr - sbuf) < ETH_ZLEN) {
        padlen = ETH_ZLEN - (ptr - sbuf);
        memset(ptr, 0, padlen);
        ptr += padlen;
    }

    write(soc, sbuf, ptr-sbuf);
    print_ether_header(eh);

    return 0;
}

int EtherRecv(int soc, u_int_8 *in_ptr, int in_len) {
    struct ether_header *eh;
    u_int8_t  *ptr = in_ptr;
    int len = in_len;

    eh = (struct ether_header *)ptr;
    ptr += sizeof(struct ether_header);
    len -= sizeof(struct ether_header);

    if(memcpy(eh->ether_dhost, BcastMac, 6) != 0 && memcmp(eh->ether_dhost, Param.vmac) != 0) {
        return -1;
    }

    if(ntohs(eh->ether_type) == ETHERTYPE_ARP) {
        ArpRecv(soc, eh, ptr, len);
    } else if(ntohs(eh->ether_type) == ETHERTYPE_IP) {
        IpRecv(soc, in_ptr, in_len, eh, ptr, len);
    }

    return 0;
}
