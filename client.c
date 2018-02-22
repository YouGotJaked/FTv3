#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include "protocol.h"
#include "protocol.c"

int main(int argc, char **argv) {
    //check argument count
    check_argc(argc, 5, argv[0]);
    
    int sockfd, port, bytes;
    struct sockaddr_in serv_addr;
    socklen_t addr_size;
    FILE *fr;
    Packet c, s;
    Packet *p_client = &c, *p_server = &s;
    (*p_client).head.chksum = 0;
    
    //initialize timer
    struct timeval tv;
    int rv;
    
    //configure address, setup select, set timer
    config_addr(serv_addr, argv[1], argv[2], addr_size, sockfd);
    
    /*
    fd_set readfds;
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    */
    
    //initialize file
    initialize_file(&fr, argv[3], "r");
    
    //initialize RNG
    srand(time(NULL));

    
    while (1) {
        //reset
        fseek(fr, 0, SEEK_SET); //hangs here
        (*p_client).head.len = 0;
        bytes = 0;
        
        //send filename and data to server	
	send_name_to_server(p_server, argv[4], sockfd, serv_addr, addr_size);        
	send_file_to_server(bytes, p_client, &fr, sockfd, serv_addr, addr_size);       
        sent_packet(p_client);
        
        printf("Client len: %d\n", (*p_client).head.len);
	
        //compute and send checksum with 20% chance to send 0
        (*p_client).head.chksum = rand() % 100 > 20 ? chksum(p_client) : 0;
        sendto(sockfd, p_client, sizeof(p_client), 0, (struct sockaddr *)&serv_addr, addr_size);
        printf("Client checksum: %d\n", (*p_client).head.chksum);
    
        //receive length and checksum from server
        recvfrom(sockfd, p_server, sizeof(p_server), 0, (struct sockaddr *)&serv_addr, &addr_size);
        printf("Server len: %d\n", (*p_server).head.len);
        printf("Server checksum: %d\n", (*p_server).head.chksum);
        
        (*p_server).head.seq_ack = (*p_server).head.len > 0 ? 1 : 0;
        
        if (ack_recv(p_server, p_client) == 1)
	    goto exit;
    }
    
    exit: 
    fclose(fr);
    close(sockfd);
    return 0;
}

