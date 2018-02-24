#include "protocol.h"

int chksum(Packet* pkt, size_t size) {
    int i;
    char *head = (char*)pkt;
    char csum = head[0];
    (*pkt).head.chksum = 0;
    
    for (i = 0; i < size; i++) {
        csum ^= head[i];
    }
    
    return (int)csum;
}

int comp_packet(Packet p1, Packet p2) {
    return p1.head.len == p2.head.len && p1.head.chksum == p2.head.chksum;
}

int micros_to_s(int s) {
    return s / 1000000;
}
