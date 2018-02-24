#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include "protocol.h"
#include "protocol.c"

int main(int argc, char **argv) {
    int sockfd, port, bytes, timeout = 0;
    struct sockaddr_in serv_addr;
    socklen_t addr_size;
    time_t t;
    Packet p_client, p_server;
    p_client.head.chksum = 0;
    
    if (argc != 5) {
        printf("Usage: %s <port number> <ip of server> <source file> <destination file>\n", argv[0]);
        return -1;
    }
    
    //configure address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
    
    if (inet_pton(AF_INET, argv[2], &serv_addr.sin_addr) <= 0) {
        perror("inet_pton error occured");
        return -1;
    }
    
    memset(serv_addr.sin_zero, '\0', sizeof(serv_addr.sin_zero));
    addr_size = sizeof(serv_addr);
    
    //create UDP socket for server
    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Could not create socket");
        return -1;
    }

    //initialize file
    FILE *fr;
    fr = fopen(argv[3], "r");
    if (fr == NULL) {
        printf("File %s not found.\n", argv[3]);
        return -1;
    }
    
    //initialize RNG
    srand((unsigned) time(&t));
    
    //set up timer
    struct timeval tv;
    int rv;
    fd_set readfds;
    
    while (1) {
        //reset
        fseek(fr, 0, SEEK_SET);
        p_client.head.len = 0;
        bytes = 0;
        
        //send filename and data to server
        strcpy(p_server.data, argv[4]);
        sendto(sockfd, (char*)&p_server, sizeof(p_server), 0, (struct sockaddr *)&serv_addr, addr_size);
        printf("Sent filename: [%s]\n",p_server.data);
        
        while ((bytes = fread(p_client.data, 1, sizeof(p_client.data), fr)) > 0) {
            p_client.head.len = bytes;
            //printf("Sent data: [%s], Packet size: [%d]\n", p_client.data, p_client.head.len);
            sendto(sockfd, (char*)&p_client, sizeof(p_client), 0, (struct sockaddr *)&serv_addr, addr_size);
        }
        
        if (p_client.head.len > 0) {
            printf("[+] Packet sent.\n");
            p_client.head.seq_ack = 1;
        } else {
            printf("[-] Packet not sent.\n");
            p_client.head.seq_ack = 0;
        }
        
        printf("Client len: %d\n", p_client.head.len);
        
        //compute and send packet with 20% chance to skip
        if (rand() % 100 > 20) {
            p_client.head.chksum = chksum(&p_client, sizeof(p_client));
            sendto(sockfd, (char*)&p_client, sizeof(p_client), 0, (struct sockaddr *)&serv_addr, addr_size);
        } else {
            printf("Packet skipped.\n");
        }
        
        printf("Client checksum: %d\n",p_client.head.chksum);
        
        //select timer
        fcntl(sockfd, F_SETFL, O_NONBLOCK);
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        
        rv = select(sockfd+1, &readfds, NULL, NULL, &tv);

        // timeout
        if (rv == 0) {
            timeout++;
            if (timeout > MAX_ATTEMPTS)
                goto timeout;
        } else if (rv == 1) {
            //receive length and checksum from server
            recvfrom(sockfd, (char*)&p_server, sizeof(p_server), 0, (struct sockaddr *)&serv_addr, &addr_size);
            printf("Server len: %d\n",p_server.head.len);
            printf("Server checksum: %d\n",p_server.head.chksum);
            p_server.head.seq_ack = p_server.head.len > 0 ? 1 : 0;
        }
        
        if (comp_packet(p_server, p_client)) {
            printf("[+] ACK1 received.\n");
            goto exit;
        }
    }
    exit: fclose(fr);
    close(sockfd);
    return 0;
    
    timeout:
    printf("Timed out after %d attempts...\n",timeout);
    return -1;
}

