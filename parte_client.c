#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>

struct timeval slotTime;
int sockfd, port, length;
struct sockaddr_in serv_addr, my_addr;
socklen_t servlen;

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

int main(int argc, char *argv[])
{
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    port = atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
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

    if (connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0) {
        printf("Error on connect\n");
        exit(1);
    }

    fd_set set, readset;
    FD_ZERO(&set);
    FD_SET(sockfd, &set);

    // done = 0;
    // timeslot = 0;
    int collisions = 0;
    int timeToNext = 1;
    
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &slotTime, sizeof(slotTime));

    int successes[21] = {0};
    int selected = 0;

    int i;
    char message[1024];

    // for (j = 0; j < 1024; j++)
    // 	message[j] = 'a';

    int lambda;
    for (lambda = 20; lambda > 3; lambda -= 2) {
        // printf("Lambda %d:\n", lambda);
    	for (i = 0; i < 5000; i++) {
            readset = set;
            timeToNext--;
            // printf("timeslot %d, timetonext %d\n", i, timeToNext);
            // if supposed to send
            if (timeToNext == 0) {
                memset(message, 'a', sizeof(message));
        		length = write(sockfd, message, sizeof(message));
        		// printf("Sending in timeslot %d\n", i);
        		if (length < 0) {
        			fprintf(stderr,"ERROR writing to socket\n");
        			exit(0);
        		}
            }

            memset(message, 0, sizeof(message));
            length = 0;
            // if sent this timeslot, wait 2x timeslots to receive response
            // if didn't send this timeslot, select will take up timeslot (timeout)
            slotTime.tv_sec = 0;
            slotTime.tv_usec = 800;
            selected = select(FD_SETSIZE, &readset, (fd_set *)0, (fd_set *)0, &slotTime);
            if (timeToNext < 1) {
                if (selected < 0)
                    printf("Error on select\n");
                // else if (selected == 0)
                //     printf("Timeout\n");
                else if (selected != 0) {
                    // printf("Message is %d bytes\n", strlen(message));
                    length = read(sockfd, &message, sizeof(message));
                    // printf("Read %d bytes\n", length);
                }
            }

            if (length > 0) {
                // printf("Message: %s\n", message);
                // if collision, compute backoff until attempt retransmission
                if (length > 8) {
                    // printf("Message: %s\n", message);
                    // printf("Collision sending in timeslot %d\n", i);
                    collisions++;
                    timeToNext = computeBackoff(collisions);
                }

                // if success, compute next transmission
                else {
                    // printf("Success sending in timeslot %d\n", i);
                    timeToNext = computeSend(lambda);
                    successes[lambda]++;
                    collisions = 0;
                }
            }

            // dunno what else happens
            else {
                if (timeToNext < 0)
                    timeToNext = 1;
                //printf("Got %d from recvfrom function in timeslot %d\n", length, i);
            }
    	}
    }

    close(sockfd);
    printf("\n");

    // print final output
    for (lambda = 20; lambda > 3; lambda -= 2) {
        printf("Lambda %2d, packets: %4d, throughput: %4d, avg delay: %3d, load: %01.2f\n", lambda, successes[lambda], successes[lambda]*1024*8/5000, 5000/successes[lambda], 2.0/(double)lambda);
    }
    return 0;
}
