#ifndef PROTOCOL_H
#define PROTOCOL_H

#define SIZE 10
#define MAX_ATTEMPTS 5
#define SLEEP 2000000

typedef struct header {
    int seq_ack;
    int len;
    int chksum;
} Header;

typedef struct packet {
    struct header head;
    char data[SIZE];
} Packet;

int chksum(Packet* pkt, size_t size);
int comp_packet(Packet p1, Packet p2);
int micros_to_s(int s);

#endif /* PROTOCOL_H */
