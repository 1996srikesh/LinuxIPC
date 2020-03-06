/*
 * Name: tcp_client.c
 *
 * Author: Srikesh Srinivas
 *
 * Date: March 5, 2020
 *
 * Description: TCP client based on network sockets
 *
 * */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <fcntl.h> // for open
#include <unistd.h> // for close

#include <netinet/in.h>

int main() {
	char *server_response = NULL;
    int connection_status = 0;
	int client_socket = 0;
    int bytes = 0;
    
    char send_msg[20] = {0};
    int len = 0;

    server_response = (char*)malloc(1024*sizeof(char));    
    if (NULL == server_response)
        goto exit;

	// create the client_socketet
	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	//setup an address
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(9002);

	connection_status = connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (connection_status == -1)
    {
        printf("Error making connection\n");
        goto exit;
	}

    bytes = recv(client_socket, server_response, 1024*sizeof(char), 0);
    if (0 >= bytes)
    {
        printf("Byte count error. Socket closed.\n");
        goto exit;
    }

    /* Print info received */
    printf("server_response is:%s and its len is : %d\n", server_response, bytes);
     
    int i = 0;
    while (i < 5)
    {
        printf("Input a message to transmit (len <=20 bytes) [client to TCP server]: \n");
        fgets(send_msg,20,stdin);
        len = strlen(send_msg);
        send(client_socket, send_msg, len, 0);
        i++;
    }
exit:
    if (server_response)
        free(server_response);

    close(client_socket);
	return 0;
}

