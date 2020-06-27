#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include "param.h"
#include "ether.h"

int EndFlag = 0;
int DeviceSoc;
PARAM param;

void *MyEthThread(void *arg) {
    int nready;
    struct pollfd targets[1];
    u_int8_t buf[2048];
    int len;

    targets[0].fd = DeviceSoc;
    targets[0].events = POLLIN|POLLERR;

    while(EndFlag == 0) {
        switch((nready=poll(targets, 1, 1000))) {
        case -1:
            if(errno != EINTR){ perror("poll"); }
            break;
        case 0:
            break;
        default:
            if(targets[0].revents & (POLLIN|POLLERR)) {
                if((len=read(DeviceSoc, buf, sizeof(buf))) <= 0) { perror("read"); }
                else { EtherRecv(DeviceSoc, buf, len); }
            }
            break;
        }
    }
    return(NULL);
}
