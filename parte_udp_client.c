#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <pthread.h>

int lost[1] = {-1};
unsigned long totalLatency[1];
struct timeval startTime, finishTime;
int sockfd, port, length;
struct sockaddr_in serv_addr, my_addr;
socklen_t servlen;

void *receiver(void *param);

int main(int argc, char *argv[])
{
    if (argc < 3) {
      // fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    port = atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
       // fprintf(stderr,"ERROR opening socket\n");
        exit(0);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(port);

    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    servlen = sizeof(serv_addr);

    int recvID = 0;
    pthread_t recvThreadID;
    pthread_attr_t recvAttr;
    pthread_attr_init(&recvAttr);
    pthread_create(&recvThreadID, &recvAttr, receiver, &recvID);

    // char message[] = malloc(KB[5] * 1024);

    int i, j;
    int rounds = 1;
    char message[1024];

    for (j = 0; j < 1024; j++)
    	message[j] = 'a';
        // start time
        gettimeofday(&startTime, NULL);

	for (j = 0; j < 100; j++) {
		length = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&serv_addr, servlen);
		printf("Sending strlen = %d\n", strlen(message));
		if (length < 0) {
			fprintf(stderr,"ERROR writing to socket\n");
			exit(0);
		}
		// printf("Sent %2dKB, cycle %3d\n", KB[i], j);
	}


    int x;
    for(x = 0; x < 20; x++){
        char endMsg[] = "End!";
        length = sendto(sockfd, endMsg, strlen(endMsg), 0, (struct sockaddr *)&serv_addr, servlen);
        printf("Sending strlen = %d\n", strlen(endMsg));
        if (length < 0) {
        fprintf(stderr,"ERROR writing to socket\n");
        exit(0);
        }
    }
    pthread_join(recvThreadID, NULL);
    for (i = 0; i < rounds; i++) {
        printf("For 1KB::  Packets lost:%2d   Total latency: %lu\nAverage latency: %lu\n", lost[i], totalLatency[i], totalLatency[i]/100);
    }
    close(sockfd);
    return 0;
}

void *receiver(void *param) {
    char message[64000];
    int length = 1024;
    int count = 0;
    int lastLength = 1024;

    while (length != 4) {
        memset(&message, 0, strlen(message));
        lastLength = length;
        length = recvfrom(sockfd, &message, sizeof(message), 0, (struct sockaddr *)&serv_addr, &servlen);
        if (length < 0) {
            fprintf(stderr,"\nERROR reading from socket\nsizeof(message)=%d\n", sizeof(message));
            exit(0);
        }

        if (length != lastLength) {
            printf("Received %d of %d size packets\n%d packets lost\n\n", count, lastLength, 100 - count);
            int i, flag = 0;
            for (i = 0; i < 1 && flag == 0; i++) {
                if (lost[i] == -1) {
                    lost[i] = 100 - count;
                    flag = 1;

                    // finish time
                    gettimeofday(&finishTime, NULL);
                    printf("For 1KB:    Start time: %lu.%lu\nFinish time: %lu.%lu\n", startTime.tv_sec, startTime.tv_usec, finishTime.tv_sec, finishTime.tv_usec);
                    unsigned long totalSec = finishTime.tv_sec - startTime.tv_sec;
                    unsigned long usec;
                    if (finishTime.tv_usec > startTime.tv_usec) {
                        usec = finishTime.tv_usec - startTime.tv_usec;
                    }
                    else {
                        usec = 1000000 - startTime.tv_usec + finishTime.tv_usec;
                    }
                    totalLatency[i] = totalSec * 1000000 + usec;
                }

            }


            count = 0;
        }

        count++;

        printf("Received %2d\n", strlen(message));
        // printf("Server response: %s\n",message);

    }
}
