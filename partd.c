#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 10
#define MAX_STATIONS 200

int min(int n, int m) {
	if (n < m)
		return n;
	return m;
}

int max(int n, int m) {
	if (n < m)
		return m;
	return n;
}

int max_array(int a[], int num_elements)
{
   int i, max = -1;
   for (i=0; i<num_elements; i++)
   {
	 if (a[i]>max)
	 {
	    max=a[i];
	 }
   }
   return(max);
}

int nextTransmission(int lambda) {
	int temp = 0;
	while(temp == 0)
		temp = (0 - lambda) * (log((double)rand()/(double)RAND_MAX) / log(2));
	return temp;
}

int computeBackoff(int n) {
	int k = min(n, 10);
	k = (int)pow(2, k);
	return rand() % k;
}

void main(int argc, char *argv[]) {
	int lambda = 200;
	int stations_ = 20;
	int p = 512;
	int defrun = 0;

	if(argc != 1 && argc != 4){
		printf("Invalid Input\n");
		exit(1);
	}

	if(argc == 4){
		lambda = atoi(argv[1]);
		defrun = atoi(argv[1]);
		stations_ = atoi(argv[2]);
		p = atoi(argv[3]);
	}


	srand(time(NULL));

	for (; lambda > 10; lambda -= 20) {

		int contentionInterval[ITERATIONS] = {0};

		int iteration;
		for (iteration = 0; iteration < ITERATIONS; iteration++) {

			// init data for iteration
			int timeSent[MAX_STATIONS] = {0};
			int lastTimeSent[MAX_STATIONS] = {0};
			int nextTimeToSend[MAX_STATIONS] = {0};
			int lastCollisionTime[MAX_STATIONS] = {0};
			int transmissionsSent[MAX_STATIONS] = {0};
			int currentCollisionCount[MAX_STATIONS] = {1};
			int lastCollisiondCount[MAX_STATIONS] = {0};

			int T = 0;

			// loop for all STATIONS to transmit
			while (T < 200000) {
				// set sending counter to 0, sendingIndex to invalid
				int sending = 0;
				int sendingIndex = -1;

				int i;

				// check how many STATIONS want to send in this interval
				for (i = 0; i < stations_; i++) {
					if (nextTimeToSend[i] == T-1 || nextTimeToSend[i] == T) {
						sending++;
						if (nextTimeToSend[i] == T)
							sendingIndex = i;
					}
				}

				// if only one STATIONS wants to send in the interval, and it wants to send
				// at time T, then send.
				if (sending == 1 && sendingIndex != -1) {
					if (nextTimeToSend[sendingIndex] == T) {
						lastTimeSent[sendingIndex] = timeSent[sendingIndex];
						timeSent[sendingIndex] = T;
						transmissionsSent[sendingIndex]++;
						lastCollisiondCount[sendingIndex] = currentCollisionCount[sendingIndex];
						currentCollisionCount[sendingIndex] = 0;
						nextTimeToSend[sendingIndex] += nextTransmission(lambda);
					}
				}
				// if more than one STATION wants to send in the interval, check if something tried
				// to send at time T-1. If so, reset it. Compute nextTimeToSend for all colliding STATION
				else if (sending > 1) {
					for (i = 0; i < stations_; i++) {
						if (timeSent[i] == T-1) {
							timeSent[i] = lastTimeSent[i];
							lastCollisionTime[i] = nextTimeToSend[i];
							currentCollisionCount[i] = lastCollisiondCount[i];
							currentCollisionCount[i]++;
							transmissionsSent[i]--;
							while (nextTimeToSend[i] <= T)
								nextTimeToSend[i] += computeBackoff(currentCollisionCount[i]);
						}
						if (nextTimeToSend[i] == T-1 || nextTimeToSend[i] == T) {
							lastCollisionTime[i] = nextTimeToSend[i];
							currentCollisionCount[i]++;
							while (nextTimeToSend[i] <= T)
								nextTimeToSend[i] += computeBackoff(currentCollisionCount[i]);
						}
					}
				}

				// increment time
				T++;
			}

			// find the max timeSent, divide by number of STATIONS to get contention interval
			int maximum = 0;
			int i;
			for (i = 0; i < stations_; i++) {
				maximum = max(maximum, timeSent[i]);
			}
			contentionInterval[iteration] = maximum / stations_;

			int z;
			printf("Time Sent:          ");
			for(z = 0; z < stations_; z++){
				printf("%6d ", timeSent[z]);
			}
			printf("\nNext Time to Send:  ");
			for(z = 0; z < stations_; z++){
				printf("%6d ", nextTimeToSend[z]);
			}
			printf("\nLast Colision Time: ");
			for(z = 0; z < stations_; z++){
				printf("%6d ", lastCollisionTime[z]);
			}
			printf("\nCollision Count:    ");
			for(z = 0; z < stations_; z++){
				printf("%6d ", currentCollisionCount[z]);
			}
			printf("\nTransmissions Sent: ");
			for(z = 0; z < stations_; z++){
				printf("%6d ", transmissionsSent[z]);
			}
			printf("\n\n");/*

			printf("Time Sent: ");
				printf("%6d ", max_array(timeSent, stations_));
			printf("\nNext Time to Send:  ");
				printf("%6d ", max_array(nextTimeToSend, stations_));
			printf("\nLast Colision Time: ");
				printf("%6d ", max_array(lastCollisionTime, stations_));
			printf("\nCollision Count:   ");
				printf("%6d ", max_array(currentCollisionCount, stations_));
			printf("\nTransmissions Sent:");
				printf("%6d ", max_array(transmissionsSent, stations_));
			printf("\n\n");*/
			//printf("Contention interval is %3d for lambda %2d iteration %3d\n", contentionInterval[iteration], lambda, iteration);
		}

		//not default run
		if(defrun != 0)
			break;

		// find minimum contention interval
		int minimum = 9999;
		int sumContention = 0;
		int i;
		for (i = 0; i < ITERATIONS; i++) {
			minimum = min(minimum, contentionInterval[i]);
			sumContention += contentionInterval[i];
		}
		printf("Lambda %2d minimum contention interval = %d\nAverage contention interval = %d\n\n", lambda, minimum, sumContention/ITERATIONS);
	}
}