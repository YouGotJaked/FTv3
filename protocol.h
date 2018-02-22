#ifndef PROTOCOL_H
#define PROTOCOL_H

#define SIZE 10

typedef struct header {
    int seq_ack;
    int len;
    int chksum;
} Header;

typedef struct packet {
    struct header head;
    char data[SIZE];
} Packet;

int ack_recv(Packet *p1, Packet *p2);
int check_argc(int argc, int n, char* exec);
int chksum(Packet *pkt);
int comp_packet(Packet *p1, Packet *p2);

void config_addr(struct sockaddr_in s, char *port, char *ip, socklen_t size, int sock);
void initialize_file(FILE **fp, char *filename, char *mode);
void send_name_to_server(Packet *pkt, char *filename, int sock, struct sockaddr_in s, socklen_t size);
void send_file_to_server(int bytes, Packet *pkt, FILE **fp, int sock, struct sockaddr_in s, socklen_t size);
void sent_packet(Packet *pkt);

#endif /* PROTOCOL_H */
