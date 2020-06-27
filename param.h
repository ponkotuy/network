#ifndef PARAMS_H
#define PARAMS_H

typedef struct {
    char *device;
    u_int8_t mymac[6];
    struct in_addr myip;
    u_int8_t vmac[6];
    struct in_addr vip;
    struct in_addr vmask;
    int IpTTL;
    int MTU;
    struct in_addr gateway;
} PARAM;

int isTargetIPAddr(struct in_addr *addr);

#endif
