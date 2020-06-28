#include <string.h>
#include "sock.h"

u_int16_t checksum(u_int_8_t *data, int len) {
    u_int32_t sum = 0;
    u_int16_t *ptr;
    int c;

    ptr = (u_int16_t *)data;
    for(c = len; 1 < c; c -= 2) {
        sum += (*ptr);
        if(sum & 0x80000000){
            sum = (sum & 0xFFFF) + (sum >> 16);
        }
        ++ptr;
    }
    if(c == 1) {
        u_int16_t val = 0;
        memcpy(&val, ptr, sizeof(u_int8_t));
        sum += val;
    }

    while(sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

u_int16_t checksum2(u_int8_t *data1, int len1, u_int8_t *data2, int len2) {
    u_int32_t sum = 0;
    u_int16_t *ptr;
    int c;

    sum = 0;
    ptr = (u_int16_t *)data1;
    for(c = len1; 1 < c; c -= 2) {
        sum += (*ptr);
        if(sum & 0x80000000) { sum = (sum & 0xFFFF) + (sum >> 16); }
        ++ptr;
    }
    if(c == 1) {
        u_int16_t val;
        val = ((*ptr << 8)) + (*data2);
        sum += val;
        if(sum & 0x80000000) { sum = (sum & 0xFFFF) + (sum >> 16); }
        ptr = (u_int16_t *)(data2 + 1);
        --len2;
    } else { ptr = (u_int16_t *)data2; }

    for(c = len2; 1 < c; c -= 2) {
        sum += (*ptr);
        if(sum & 0x80000000) { sum = (sum & 0xFFFF) + (sum >> 16); }
        ++ptr;
    }
    if(c == 1) {
        u_int16_t val = 0;
        memcpy(&val, ptr, sizeof(u_int8_t));
        sum += val;
    }

    while(sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}
