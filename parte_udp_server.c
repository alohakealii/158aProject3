#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct timeval slotTime;
char message[1024] = {0};
char message2[1024] = {0};
int sockfd, length2;
struct sockaddr_in cli2_addr;

void *receiver(void *param) {
    memset(message2, 0, sizeof(message2));
    length2 = recvfrom(sockfd, &message2, sizeof(message2), 0, (struct sockaddr *) &cli2_addr, &sizeof(cli2_addr));
    if (length2 < 0) {
        printf("receiver thread error %d", length2);
    }
}

int main(int argc, char *argv[])
{
    int port;
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
    
    int i;
    time_t t1, t2;
    int length = 0;
    
    //set slot times
    slotTime.tv_sec = 0;
    slotTime.tv_usec = 800;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &slotTime, sizeof(slotTime));

    int recvID = 0;
    pthread_t recvThreadID;
    pthread_attr_t recvAttr;
    pthread_attr_init(&recvAttr);
    
    while (true) {
        // thread receives the from the second client
        pthread_create(&recvThreadID, &recvAttr, receiver, &recvID);
        
        // receive from the first client
        memset(message, 0, sizeof(message));
        length = recvfrom(sockfd, &message, sizeof(message), 0, (struct sockaddr *) &cli_addr, &sizeof(cli_addr));
        if (length < 0) {
            printf("receiver thread error %d", length);
        }

        // make sure both receives are done
        pthread_join(recvThreadID, NULL);

        // check how many clients received packets from
        // if two clients sent, collision
        if (!length && !length2) {
            memset(message, 0, sizeof(message));
            message = "Collision";
            length = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *) &cli_addr, sizeof(cli_addr));
            if (length < 0)
                printf("Error sending to cli_addr\n");

            length = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *) &cli2_addr, sizeof(cli2_addr));
            if (length < 0)
                printf("Error sending to cli2_addr\n");
        }

        // if one client sent, success
        else if (!length || !length2) {
            memset(message, 0, sizeof(message));
            message = "Success";
            if (!length)
                length = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *) &cli_addr, sizeof(cli_addr));
            else
                length = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *) &cli2_addr, sizeof(cli2_addr));
            if (length < 0)
                printf("error sending success message\n");
        }

        // zero message buffers
        memset(message, 0, sizeof(message));
        memset(message2, 0, sizeof(message2));
    }
    
    close(sockfd);
    return 0;
}
