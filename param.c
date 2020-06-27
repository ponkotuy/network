#include <arpa/inet.h>

extern PARAM Param;

int isTargetIPAddr(struct in_addr *addr) {
    return Param.vip.s_addr == addr->s_addr : 1 ? 0;
}
