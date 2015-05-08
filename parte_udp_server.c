#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
    int sockfd, port;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)  {
        fprintf(stderr,"ERROR opening socket\n");
        exit(1);
    }
    port = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr,"ERROR on binding\n");
        exit(1);
    }

    socklen_t clilen = sizeof(cli_addr);

    // int rounds = 6;

    // int KB[6] = {1, 4, 8, 16, 32, 64};
    char message[64000];

    int i;
    while (strlen(message) != 42) {
        memset(&message, 0, strlen(message));
    // printf("About to realloc to %2dKB\n", KB[i]);
    // realloc(message, KB[i] * 1024);
    // printf("Realloc'd successfully\n");
        n = recvfrom(sockfd, message, sizeof(message), 0, (struct sockaddr *)&cli_addr, &clilen);
        if (n < 0) {
            fprintf(stderr,"ERROR reading from socket\n");
            exit(1);
        }
        //get time received; check if within 1 time slot
        //if within 1 time slot, collision. else success
        printf("Received strlen(message) = %d\n", strlen(message));

        // printf("Received %2dKB, cycle %3d\n", KB[i], j);
        // printf("Here is the message: %s\n", message);

        n = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&cli_addr, clilen);
        // printf("Sent %2dKB, cycle %3d\n", strlen(message), j);
        if (n < 0) {
            fprintf(stderr,"ERROR writing to socket\n");
            exit(1);
        }
    }

    close(sockfd);
    return 0;
}
