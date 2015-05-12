#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_CONNECTIONS 2

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)  {
        fprintf(stderr,"ERROR opening socket\n");
        exit(1);
    }
    printf("sockfd = %d\n", sockfd);

    // allow multiple connections to the server address
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        fprintf(stderr, "ERROR setsockopt reuseaddr\n");
        exit(1);
    }

    // setup address
    int port = atoi(argv[1]);
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(sockfd, (struct sockaddr *) &address, addr_len) < 0) {
        fprintf(stderr,"ERROR on binding\n");
        exit(1);
    }

    // listen for MAX_CONNECTIONS connections
    if (listen(sockfd, MAX_CONNECTIONS) < 0) {
        fprintf(stderr, "ERROR listen\n");
        exit(1);
    }

    // socket data
    fd_set readfds;
    int client_socket[MAX_CONNECTIONS] = {0};
    int max_sd, sd, new_sd;

    // message buffers
    char *message = malloc(1025);
    char *message2 = malloc(1025);
    memset(message, 0, sizeof(message));
    memset(message2, 0, sizeof(message2));
    
    // other variables used in infinite loop
    int length = 0;
    int value;
    int i;
    struct timeval slotTime;

    printf("Entering infinite loop\n");
    while (1) {        

        // clear and add main sockfd to set
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        max_sd = sockfd;

        // add client sockets to set
        for ( i = 0 ; i < MAX_CONNECTIONS ; i++) 
        {
            sd = client_socket[i];
             
            if(sd > 0)
                FD_SET(sd , &readfds);
             
            if(sd > max_sd)
                max_sd = sd;
        }

        slotTime.tv_sec = 2;
        slotTime.tv_usec = 800;

        // for a slot time, wait to check if something happened on a socket
        value = select(FD_SETSIZE, &readfds, NULL, NULL, &slotTime);

        printf("Selected\n");

        if (value < 0 && errno != EINTR) 
            printf("ERROR select\n");

        // if something happened on sockfd, it is a new connection
        if (FD_ISSET(sockfd, &readfds)) {
            if (new_sd = accept(sockfd, (struct sockaddr *) &address, &addr_len) < 0) {
                fprintf(stderr, "ERROR accept\n");
                exit(1);
            }

            printf("New connection, descriptor is %d, ip is %s, port is %d\n", new_sd, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // add new socket to array
            for (i = 0; i < MAX_CONNECTIONS; i++) 
            {
                //if position is empty
                if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_sd;
                    printf("Adding to list of sockets as %d\n" , i);                     
                }
            }
        }

        // if a client socket has data
        if (FD_ISSET(client_socket[0], &readfds)) {
            length = read(client_socket[0], message, sizeof(message));
            if (length < 0) {
                fprintf(stderr, "ERROR read client 0\n");
            }
            else {
                printf("Read %d bytes from client 0\n", length);
            }
        }

        if (FD_ISSET(client_socket[1], &readfds)) {
            length = read(client_socket[1], message2, sizeof(message2));
            if (length < 0) {
                fprintf(stderr, "ERROR read client 1\n");
            }
            else {
                printf("Read %d bytes from client 1\n", length);
            }
        }

        // if received from both clients
        if (strlen(message) > 0 && strlen(message2) > 0) {
            memset(message, 0, sizeof(message));
            strncpy(message, "Collision", 9);
            length = write(client_socket[0], message, strlen(message));
            if (length < 0) {
                fprintf(stderr, "ERROR write client 0\n");
            }
            length = write(client_socket[1], message, strlen(message));
            if (length < 0) {
                fprintf(stderr, "ERROR write client 1\n");
            }
        }

        // if received from one client
        else if (strlen(message) || strlen(message2)) {
            if (strlen(message) > 0) {
                memset(message, 0, sizeof(message));
                strncpy(message, "Success", 7);
                length = write(client_socket[0], message, strlen(message));
                if (length < 0) {
                    fprintf(stderr, "ERROR write client 0\n");
                }
            }
            else {
                memset(message2, 0, sizeof(message2));
                strncpy(message2, "Success", 7);
                length = write(client_socket[1], message2, strlen(message2));
                if (length < 0) {
                    fprintf(stderr, "ERROR write client 1\n");
                }
            }
        }

        // clear message buffers
        memset(message, 0, sizeof(message));
        memset(message2, 0, sizeof(message2));
    }
    
    close(sockfd);
    return 0;
}
