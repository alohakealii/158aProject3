#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define MAX_CONNECTIONS 2

void readmsg(int sockfd, char *message, char *buffer, int size) {
    int n;
    while (strlen(message) < size) {
        n = read(sockfd, buffer, 1024);
        if (n < 0) 
             error("ERROR reading from socket");
        // if (n == 0)
        //     printf("read == 0\n");

        // if there are more bytes than fit for the current message
        if (strlen(message) + strlen(buffer) > size) {
            int amount = size - strlen(message); // calculate bytes for current message
            int i;

            // copy remaining byes for current message
            for (i = 0; i < amount; i++) {
                message[strlen(message)] = buffer[i];
            }

            // zero the rest of the buffer, leaving the excess belonging to next message
            memset(buffer + amount, 0, 1024 - amount);
        }
        else {
            strcat(message, buffer);
            memset(buffer, 0, 1024);
        }
    }
    // printf("length = %d   strlen(message) = %d   size = %d\n", n, strlen(message), size);
}

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

    // client address
    struct sockaddr_in cli_addr;
    socklen_t clilen;
    
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
    int client_socket[MAX_CONNECTIONS];
    int i;
    for (i = 0; i < MAX_CONNECTIONS; i++)
        client_socket[i] = 0;
    int max_sd, sd, new_sd;

    // message buffers
    char *message = malloc(1024);
    char *message2 = malloc(1024);
    char *buffer = malloc(1024);
    memset(message, 0, 1024);
    memset(message2, 0, 1024);
    memset(buffer, 0, 1024);
    
    // other variables used in infinite loop
    int length = 0;
    int value;
    struct timeval slotTime;

    // printf("Entering infinite loop\n");
    while (1) {        

        // clear and add main sockfd to set
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        // printf("Added sockfd to FD_SET\n");

        max_sd = sockfd;

        // add client sockets to set
        for ( i = 0 ; i < MAX_CONNECTIONS ; i++) 
        {
            sd = client_socket[i];
             
            if(sd > 0) {
                FD_SET(sd , &readfds);
                // printf("Added socket %d to FD_SET\n", sd);
            }
             
            if(sd > max_sd)
                max_sd = sd;
        }

        slotTime.tv_sec = 0;
        slotTime.tv_usec = 800;

        // for a slot time, wait to check if something happened on a socket
        value = select(FD_SETSIZE, &readfds, NULL, NULL, &slotTime);

        // printf("Selected %d\n", value);

        if (value < 0 && errno != EINTR) 
            printf("ERROR select\n");

        // if something happened on sockfd, it is a new connection
        if (FD_ISSET(sockfd, &readfds)) {
            new_sd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
            if (new_sd < 0) {
                fprintf(stderr, "ERROR accept\n");
                exit(1);
            }

            // printf("New connection, descriptor is %d, ip is %s, port is %d\n", new_sd, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

            int flag = 0;
            // add new socket to array
            for (i = 0; i < MAX_CONNECTIONS; i++) 
            {
                //if position is empty
                if( client_socket[i] == 0 && flag == 0 )
                {
                    client_socket[i] = new_sd;
                    // printf("Adding socket %d to list of sockets as %d\n", new_sd ,i); 
                    flag = 1;
                }
            }
        }

        // if a client socket has data
        if (FD_ISSET(client_socket[0], &readfds)) {
            // printf("Data from client_socket[0]\n");
            memset(buffer, 0, 1024);
            readmsg(client_socket[0], message, buffer, 1024);
            // printf("Read %d bytes from client 0\n", strlen(message));
        }

        if (FD_ISSET(client_socket[1], &readfds)) {
            // printf("Data from client_socket[1]\n");
            memset(buffer, 0, 1024);
            readmsg(client_socket[1], message2, buffer, 1024);
            // printf("Read %d bytes from client 1\n", strlen(message2));
        }

        // if received from both clients
        if (strlen(message) > 0 && strlen(message2) > 0) {
            memset(message, 0, 1024);
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
                memset(message, 0, 1024);
                strncpy(message, "Success", 7);
                length = write(client_socket[0], message, strlen(message));
                if (length < 0) {
                    fprintf(stderr, "ERROR write client 0\n");
                }
            }
            else {
                memset(message2, 0, 1024);
                strncpy(message2, "Success", 7);
                length = write(client_socket[1], message2, strlen(message2));
                if (length < 0) {
                    fprintf(stderr, "ERROR write client 1\n");
                }
            }
            // printf("Wrote to client\n");
        }

        // clear message buffers
        memset(message, 0, 1024);
        memset(message2, 0, 1024);
    }
    
    close(sockfd);
    return 0;
}
