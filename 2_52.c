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
	return (0 - lambda) * (log((double)rand()/(double)RAND_MAX) / log(2));
}

int done(int *transmitted) {
	int i;
	for (i = 0; i < STATIONS; i++) {
		if (!transmitted[i]) {
			return 0;
		}
	}
	return 1;
}

void main(int argc, char *argv[]) {
	srand(time(NULL));

	int lambda;
	for (lambda = 20; lambda > 3; lambda -= 2) {

		int contentionInterval[ITERATIONS] = {0};

		int iteration;
		for (iteration = 0; iteration < ITERATIONS; iteration++) {

			// init data for iteration
			int timeSent[STATIONS] = {0};
			int nextTimeToSend[STATIONS] = {0};
			int lastAttemptTime[STATIONS] = {0};
			int T = 0;

			// loop for all stations to transmit
			while (!done(timeSent)) {

				// set sending counter to 0, sendingIndex to invalid
				int sending = 0;
				int sendingIndex = -1;

				int i;

				// check how many stations want to send in this interval
				for (i = 0; i < STATIONS; i++) {
					if (nextTimeToSend[i] == T-1 || nextTimeToSend[i] == T) {
						sending++;
						if (nextTimeToSend[i] == T)
							sendingIndex = i;
					}
				}

				// if only one station wants to send in the interval, and it wants to send
				// at time T, then send.
				if (sending == 1 && sendingIndex != -1) {
					if (nextTimeToSend[sendingIndex] == T) {
						timeSent[sendingIndex] = T;
					}
				}
				// if more than one station wants to send in the interval, check if something tried
				// to send at time T-1. If so, reset it. Compute nextTimeToSend for all colliding stations
				else if (sending > 1) {
					for (i = 0; i < STATIONS; i++) {
						if (timeSent[i] == T-1) {
							timeSent[i] = 0;
							lastAttemptTime[i] = nextTimeToSend[i];
							while (nextTimeToSend[i] <= T)
								nextTimeToSend[i] = T + computeBackoff(lambda);
						}
						if (nextTimeToSend[i] == T-1 || nextTimeToSend[i] == T) {
							lastAttemptTime[i] = nextTimeToSend[i];
							while (nextTimeToSend[i] <= T)
								nextTimeToSend[i] = T + computeBackoff(lambda);
						}
					}
				}

				// increment time
				T++;
			}

			// find the max timeSent, divide by number of stations to get contention interval
			int maximum = 0;
			int i;
			for (i = 0; i < STATIONS; i++) {
				maximum = max(maximum, timeSent[i]);
			}
			contentionInterval[iteration] = maximum / STATIONS;
			//printf("Contention interval is %3d for lambda %2d iteration %3d\n", contentionInterval[iteration], lambda, iteration);
		}

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
