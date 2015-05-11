#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>
#include <string.h>

// unsigned long totalLatency[1];
struct timeval slotTime;
int sockfd, port, length;
struct sockaddr_in serv_addr, my_addr;
socklen_t servlen;
// int done;
// int timeslot;

int min(int n, int m) {
    if (n < m)
        return n;
    return m;
}

int computeBackoff(int n) {
    int k = min(n, 10);
    k = (int)pow(2, k);
    return rand() % k;
}

int computeSend(int lambda) {
    return (0 - lambda) * (log((double)rand()/(double)RAND_MAX) / log(2));
}

// void *receiver(void *param) {
//     char message[1024];
//     int length = 1024;
//     int successes = 0;
//     int collisions = 0;

//     while (!done) {
//         memset(&message, 0, strlen(message));
//         lastLength = length;
//         length = recvfrom(sockfd, &message, sizeof(message), 0, (struct sockaddr *)&serv_addr, &servlen);
//         if (length < 0) {
//             fprintf(stderr,"\nERROR reading from socket\nsizeof(message)=%d\n", sizeof(message));
//             exit(0);
//         }

//         if (length != lastLength) {
//             printf("Received %d of %d size packets\n%d packets lost\n\n", count, lastLength, 100 - count);
//             int i, flag = 0;
//             for (i = 0; i < 1 && flag == 0; i++) {
//                 if (lost[i] == -1) {
//                     lost[i] = 100 - count;
//                     flag = 1;

//                     // finish time
//                     gettimeofday(&finishTime, NULL);
//                     printf("For 1KB:    Start time: %lu.%lu\nFinish time: %lu.%lu\n", startTime.tv_sec, startTime.tv_usec, finishTime.tv_sec, finishTime.tv_usec);
//                     unsigned long totalSec = finishTime.tv_sec - startTime.tv_sec;
//                     unsigned long usec;
//                     if (finishTime.tv_usec > startTime.tv_usec) {
//                         usec = finishTime.tv_usec - startTime.tv_usec;
//                     }
//                     else {
//                         usec = 1000000 - startTime.tv_usec + finishTime.tv_usec;
//                     }
//                     totalLatency[i] = totalSec * 1000000 + usec;
//                 }

//             }


//             count = 0;
//         }

//         count++;

//         printf("Received %2d\n", strlen(message));
//         // printf("Server response: %s\n",message);

//     }
// }

int main(int argc, char *argv[])
{
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    port = atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr,"ERROR opening socket\n");
        exit(0);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(port);

    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    servlen = sizeof(serv_addr);

    // done = 0;
    // timeslot = 0;
    int collisions = 0;
    int timeToNext = 1;
    slotTime.tv_sec = 0;
    slotTime.tv_usec = 800;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &slotTime, sizeof(slotTime));

    int successes[20] = {0};

    // int recvID = 0;
    // pthread_t recvThreadID;
    // pthread_attr_t recvAttr;
    // pthread_attr_init(&recvAttr);
    // pthread_create(&recvThreadID, &recvAttr, receiver, &recvID);


    int i;
    char message[1024];

    // for (j = 0; j < 1024; j++)
    // 	message[j] = 'a';

    int lambda;
    for (lambda = 20; lambda > 3; lambda -= 2) {
        printf("Lambda %d:\n", lambda);
    	for (i = 0; i < 5000; i++) {
            timeToNext--;

            // if supposed to send
            if (timeToNext < 1) {
                memset(message, 'a', sizeof(message));
        		length = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&serv_addr, servlen);
        		printf("Sending in timeslot %d\n", i);
        		if (length < 0) {
        			fprintf(stderr,"ERROR writing to socket\n");
        			exit(0);
        		}
            }

            memset(message, 0, sizeof(message));
            // if sent this timeslot, wait 2x timeslots to receive response
            if (timeToNext < 1) {
                length = recvfrom(sockfd, &message, sizeof(message), 0, (struct sockaddr *)&serv_addr, &servlen);
                if (length < 1) {
                    length = recvfrom(sockfd, &message, sizeof(message), 0, (struct sockaddr *)&serv_addr, &servlen);
                    i++;
                }
            }

            // if didn't send this timeslot, receive to take up timeslot (timeout)
            else {
                length = recvfrom(sockfd, &message, sizeof(message), 0, (struct sockaddr *)&serv_addr, &servlen);
            }

            if (length > 0) {
                // if collision, compute backoff until attempt retransmission
                if (length == 9) {
                    printf("Collision sending in timeslot %d\n", i);
                    collisions++;
                    timeToNext = computeBackoff(collisions);
                }

                // if success, compute next transmission
                else {
                    printf("Success sending in timeslot %d\n", i);
                    timeToNext == computeSend(lambda);
                    successes[lambda]++;
                    collisions = 0;
                }
            }

            // dunno what else happens
            else {
                printf("Got %d from recvfrom function in timeslot %d\n", length, i);
            }
    	}
    }

    // done = 1;

    // pthread_join(recvThreadID, NULL);
    close(sockfd);
    printf("\n");

    // print final output
    for (lambda = 20; lambda > 3; lambda -= 2) {
        printf("Lambda %2d, packets transmitted: %4d\n", lambda, successes[lambda]);
    }
    return 0;
}
