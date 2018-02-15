#include "protocol.h"

/*
int chksum(Packet* pkt) {
  int i;
  char csum = 0;
  for (i = 0; i < len(pkt); i++) {
    csum ^= pkt[i];
  }
  return (int)csum;
}
*/

int quit() {
    return -1;
}

int ack_recv(Packet p1, Packet p2) {
    if (comp_packet(p1, p2)) {
	printf("[+] ACK1 received.\n");
	return 1;
    }
    printf("[-] ACK0 received.\n");
    return 0;
}

int check_argc(int argc, int n, char* exec) {
    if (argc != n) {
	printf("usage: %s <port number> <server ip> <source file> <destination file>\n", exec);
	return -1;
    }
    return 1;
}

int chksum(Packet *pkt, size_t size) {
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

void config_addr(struct sockaddr_in s, char *port, char *ip, socklen_t size, int sock) {
    s.sin_family = AF_INET;
    s.sin_port = htons(atoi(port));
    
    if (inet_pton(AF_INET, ip, &s.sin_addr) <= 0) {
	perror("inet_pton error occured");
	quit();
    }
    
    memset(s.sin_zero, '\0', sizeof(s.sin_zero));
    size = sizeof(s);
    
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
	perror("Could not create socket");
	quit();
    }
}

void initialize_file(FILE **fp, char *filename, char *mode) {
    if ((*fp = fopen(filename, mode)) == NULL) {
	printf("File %s not found.\n", filename);
	quit();
    }
}

void send_name_to_server(Packet pkt, char *filename, int sock, struct sockaddr_in s, socklen_t size) {
    strcpy(pkt.data, filename);
    sendto(sock, (char*)&pkt, sizeof(pkt), 0, (struct sockaddr *)&s, size);
    printf("Sent filename: [%s]\n", pkt.data);
}
  
void send_file_to_server(int bytes, Packet pkt, FILE **fp, int sock, struct sockaddr_in s, socklen_t size) {
    while ((bytes = fread(pkt.data, 1, sizeof(pkt), *fp)) > 0) { //hangs here
	pkt.head.len = bytes;
	printf("Sent data: [%s], Packet size: [%d]\n", pkt.data, pkt.head.len);
	sendto(sock, (char*)&pkt, sizeof(pkt), 0, (struct sockaddr *)&s, size);
    }
}

void sent_packet(Packet pkt) {
    if (pkt.head.len > 0) {
	printf("[+] Packet sent.\n");
	pkt.head.seq_ack = 1;
    } else {
	printf("[-] Packet not sent.\n");
	pkt.head.seq_ack = 0;
    }
}

