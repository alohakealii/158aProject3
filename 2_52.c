#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 50
#define STATIONS 20

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

int computeBackoff(int lambda) {
	return (0 - lambda) * log((double)rand() / (double)RAND_MAX);
}

int done(int *transmitted, int num_of_stations) {
	int i;
	for (i = 0; i < num_of_stations; i++) {
		if (!transmitted[i]) {
			return 0;
		}
	}
	return 1;
}

void main(int argc, char *argv[]) {
	srand(time(NULL));

	int lambda;
	for (lambda = 20; lambda >= 0; lambda -= 2) {

		int contentionInterval[ITERATIONS] = {0};

		int iteration;
		for (iteration = 0; iteration < ITERATIONS; iteration++) {

			// init data for iteration
			int timeSent[STATIONS] = {0};
			int nextTimeToSend[STATIONS] = {0};
			int lastAttemptTime[STATIONS] = {0};
			int T = 0;

			// loop for all stations to transmit
			while (!done(timeSent, STATIONS)) {

				// set sending counter to 0, sendingIndex to invalid
				int sending = 0;
				int sendingIndex = -1;

				int i;

				// check how many stations want to send in this interval
				for (i = 0; i < STATIONS; i++) {
					if (nextTimeToSend[i] == T-1 || nextTimeToSend[i] == T) {
						sending++;
						sendingIndex = i;
					}
				}

				// if only one stations wants to send in the interval, and it wants to send
				// at time T, then send.
				if (sending == 1) {
					if (nextTimeToSend[sendingIndex] == T) {
						timeSent[sendingIndex] == T;
					}
				}
				// if more than one station wants to send in the interval, check if something tried
				// to send at time T-1. If so, reset it. Compute nextTimeToSend for all colliding stations
				else if (sending > 1) {
					
				}

				// increment time
				T++;
			}
		}
	}
}
