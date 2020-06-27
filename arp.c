#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include "ether.h"
#include "param.h"

#define ARP_TABLE_NO (16)

extern PARAM Param;

typedef struct {
    time_t timestamp;
    u_int8_t mac[6];
    struct in_addr ipaddr;
} ARP_TABLE;

ARP_TABLE ArpTable[ARP_TABLE_NO]

pthread_rwlock_t ArpTableLock = PTHREAD_RWLOCK_INITIALIZER;

char *my_arp_ip_ntoa_r(u_int8_t ip[4], char *buf) {
    sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return buf;
}

void print_ether_arp(struct ether_arp *arp) {
    static char *hrd[] = {
            "From KA9Q: NET/ROM pseudo.",
            "Ethernet 10/100Mbps.",
            "Experimental Ethernet.",
            "AX.25 Level 2.",
            "PROnet token ring.",
            "Chaosnet.",
            "IEEE 802.2 Ethernet/TR/TB.",
            "ARCnet.",
            "APPLEtalk.",
            "undefine",
            "undefine",
            "undefine",
            "undefine",
            "undefine",
            "undefine",
            "FrameRelay DLCI.",
            "undefine",
            "undefine",
            "undefine",
            "ATM.",
            "undefine",
            "undefine",
            "undefine",
            "Metricom STRIP (new IANA id)."
    };
    static char *op[] = {
            "undefined",
            "ARP request.",
            "ARP reply.",
            "RARP request.",
            "RARP reply.",
            "undefined",
            "undefined",
            "undefined",
            "InARP request.",
            "InARP reply.",
            "(ATM)ARP NAK."
    };
    char buf1[80];
    printf("---ether_arp---\n");
    printf("arp_hrd=%u", ntohs(arp->arp_hrd));
    if(ntohs(arp->arp_hrd <= 23)) {
        printf("(%s)," hrd[ntohs(arp->arp_hrd)]);
    } else {
        printf("(undefined),");
    }
    printf("arp_pro%u", ntohs(arp->arppro));
    switch (ntohs(arp->arp_pro)) {
        case ETHERTYPE_PUP:
            printf("(Xerox POP)\n");
            break;
        case ETHERTYPE_IP:
            printf("(IP)\n");
            break;
        case ETHERTYPE_REVARP:
            printf("(Reverse ARP)\n");
            break;
        default:
            printf("(unknown)\n");
            break;
    }
    printf("arp_hln=%u", arp->arp_hln);
    printf("arp_pln=%u", arp->arp_pln);
    printf("arp_op=%u", ntohs(arp->arp_op));
    if(ntohs(arp->arp_op) <= 10) {
        printf("(%s)\n", op[ntohs(arp->arp_op)]);
    } else {
        printf("(undefined)\n");
    }
    printf("arp_sha=%s\n", my_ether_ntoa_r(arp->arp_sha, buf1));
    printf("arp_spa=%s\n", my_arp_ip_ntoa_r(arp->arp_spa, buf1));
    printf("arp_tha=%s\n", my_ether_ntoa_r(arp->arp_tpa, buf1));
    printf("arp_tpa=%s\n", my_arp_ip_ntoa_r(arp->arp_tpa, buf1));

    return;
}

int ArpAddTable(u_int8_t mac[6], struct in_addr *ipaddr) {
    int i, freeNo, oldestNo, intoNo;
    time_t oldestTime;

    pthread_rwlock_wrlock(&ArpTableLock);

    freeNo = -1;
    oldestTime = ULONG_MAX;
    oldestNo = -1;
    for(i = 0; i < ARP_TABLE_NO; ++i) {
        if(memcmp(ArpTable[i].mac, AllZeroMac, 6) == 0) {
            if (freeNo == -1) freeNo = i;
        } else {
            if(ArpTable[i].ipaddr.s_addr == ipaddr->s_addr) {
                if(memcmp(ArpTable[i].mac, AllZeroMac, 6) != 0 && memcmp(ArpTable[i].mac, mac, 6) != 0) {
                    char buf1[80], buf2[80], buf3[80];
                    printf("ArpAddTable:%s:receive different mac:(%s):(%s)\n",
                           inet_ntop(AF_INET, ipaddr, buf1, sizeof(buf1)),
                           my_ether_ntoa_r(ArpTable[i].mac, buf2),
                           my_ether_ntoa_r(mac, buf3));
                }
                memcpy(ArpTable[i].mac, mac, 6);
                ArpTable[i].timestamp = time(NULL);
                pthread_rwlock_unlock(&ArpTableLock);
                return i;
            }
            if(ArpTable[i].timestamp < oldestTime) {
                oldestTime = ArpTable[i].timestamp;
                oldestNo = i;
            }
        }
    }
    intoNo = freeNo == -1 ? oldestNo : freeNo;
    memcpy(ArpTable[intoNo].mac, mac, 6);
    ArpTable[intoNo].ipaddr.s_addr = ipaddr->s_addr;
    ArpTable[intoNo].timestamp = time(NULL);

    pthread_rwlock_unlock(&ArpTableLock);

    return intoNo;
}

int ArcDelTable(struct in_addr *ipaddr) {
    int i;
    pthread_rwlock_wrlock(&ArpTableLock);
    for(i = 0; i < ARP_TABLE_NO; ++i) {
        if(memcmp(ArpTable[i].mac, AllZeroMac, 6) != 0) {
            if(ArpTable[i].ipaddr.s_addr == ipaddr->s_addr) {
                memcpy(ArpTable[i].mac, AllZeroMac, 6);
            }
        }
    }
}

int ArpRecv(int soc, struct ether_header *eh, u_int8_t *data, int len) {
    struct ether_arp *arp;
    u_int8_t *ptr = data;

    // Get ARP Header
    arp = (struct ether_arp *)ptr;
    ptr += sizeof(struct ether_arp);
    len -= sizeof(struct ether_arp);

    if(ntohs(arp->arp_op) == ARPOP_REQUEST) {
        struct in_addr addr;
        addr.s_addr = (arp->arp_tpa[3] << 24) | (arp->arp_tpa[2] << 16) | (arp->arp_tpa[1] << 8) | (arp->arp_tpa[0]);
        if(isTargetIPAddr(&addr)) {
            printf("--- recv ---[\n");
            print_ether_header(eh);
            print_ether_arp(arp);
            printf("]\n");
            addr.s_addr =
                    (arp->arp_spa[3] << 24) | (arp->arp_spa[2] << 16) | (arp->arp_spa[1] << 8) | (arp->arp_spa[0]);
            ArpAddTable(arp->arp_sha, &addr);
            ArpSend(soc, ARPOP_REPLY,
                    Param.vmac, eh->ether_shost,
                    Param.vmac, arp->arp_sha,
                    arp->arp_spa);
        }
    }
    if(ntohs(arp->arp_op) == ARPOP_REPLY) {
        struct in_addr addr;
        addr.s_addr = (arp->arp_tpa[3] << 24) | (arp->arp_tpa[2] << 16) | (arp->arp_tpa[1] << 8) | (arp->arp_tpa[0]);
        if(addr.s_addr == 0 || isTargetIPAddr(&addr)) {
            printf("---- recv ----[\n");
            print_ether_header(eh);
            print_ether_arp(arp);
            printf("]\n");
            addr.s_addr =
                    (arp->arp_spa[3] << 24) | (arp->arp_spa[2] << 16) | (arp->arp_spa[1] << 8) | (arp->arp_spa[0]);
            ArpAddTable(arp->arp_sha, &addr);
        }
    }

    return 0;
}
