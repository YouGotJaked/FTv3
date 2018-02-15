/* 
 * File:   client.c
 * Author: jakeday
 *
 * Created on February 1, 2018, 9:18 AM
 */

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

/*
 *  include socket library
 *  create UDP socket for server
 *  get user keyboard input
 *  attach server name, port to message; send into socket
 *  read reply char from socket into string
 *  print out received string and close socket
 */

int main(int argc, char **argv) {
    int sockfd, port, bytes;
    struct sockaddr_in serv_addr;
    socklen_t addr_size;
    FILE *fr;
    time_t t;
    Packet p_client, p_server;
    p_client.head.chksum = 0;
    
    //check argument count
    if (check_argc(argc, 5, argv[0]) != 1)
	return -1;
    
    //configure address
    config_addr(serv_addr, argv[1], argv[2], addr_size, sockfd);
    
    //initialize file
    initialize_file(&fr, argv[3], "r");
    
    //initialize RNG
    srand((unsigned) time(&t));

    
    while (1) {
        //reset
        fseek(fr, 0, SEEK_SET); //hangs here
        p_client.head.len = 0;
        bytes = 0;
        
        //send filename and data to server	
	send_name_to_server(p_server, argv[4], sockfd, serv_addr, addr_size);        
	send_file_to_server(bytes, p_client, &fr, sockfd, serv_addr, addr_size);       
        sent_packet(p_client);
        
        printf("Client len: %d\n", p_client.head.len);
	
        //compute and send checksum with 10% chance to send 0
        p_client.head.chksum = rand() % 100 > 10 ? chksum(&p_client, sizeof(p_client)) : 0;
        sendto(sockfd, (char*)&p_client, sizeof(p_client), 0, (struct sockaddr *)&serv_addr, addr_size);
        printf("Client checksum: %d\n",p_client.head.chksum);
    
        //receive length and checksum from server
        recvfrom(sockfd, (char*)&p_server, sizeof(p_server), 0, (struct sockaddr *)&serv_addr, &addr_size);
        printf("Server len: %d\n",p_server.head.len);
        printf("Server checksum: %d\n",p_server.head.chksum);
        
        p_server.head.seq_ack = p_server.head.len > 0 ? 1 : 0;
        
        if (ack_recv(p_server, p_client) == 1)
	    goto exit;
    }
    
    exit: 
    fclose(fr);
    close(sockfd);
    return 0;
}

