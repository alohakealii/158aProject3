#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ITERATIONS 100
#define MAX_STATIONS 100

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

int computeBackoff(int n) {
	int k = min(n, 10);
	k = (int)pow(2, k);
	return rand() % k;
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
	if (argc != 2) {
		fprintf(stderr, "Run with number of stations as an argument.\n  For example: \"main.exe 20\"\n");
		exit(1);
	}

	int STATIONS = atoi(argv[1]);

	srand(time(NULL));
	int first[ITERATIONS], second[ITERATIONS], last[ITERATIONS];
	int iteration;
	for (iteration = 0; iteration < ITERATIONS; iteration++) {

		// init data for iteration
		int timeSent[MAX_STATIONS] = {0};
		int nextTimeToSend[MAX_STATIONS] = {0};
		int collisionCount[MAX_STATIONS] = {0};
		int T = 0;

		// loop for all stations to transmit
		while (!done(timeSent, STATIONS)) {

			// set sending counter to 0, collision to false
			int sending = 0;
			int collision = 0;

			int sendingIndex = -1;

			int i;
			// check each station
			for (i = 0; i < STATIONS; i++) {
				int j;
				// check if this is the only station trying to send
				for (j = 0; j < STATIONS; j++)
					if (nextTimeToSend[j] == T) {
						sending++;
						sendingIndex = j;
					}

				// set timeSent if only one trying to send, else set new times to send
				if (sending == 1) {
					timeSent[sendingIndex] = T;
				}
				else if (sending > 1) {
					for (j = 0; j < STATIONS; j++)
						if (nextTimeToSend[j] == T) {
							collisionCount[j]++;
							nextTimeToSend[j] = T + computeBackoff(collisionCount[j]);
						}
				}
			}

			// increment time
			T++;
		}

		int minimum = 9999;
		int i;

		// calculate delay of first transmitted station
		for (i = 0; i < STATIONS; i++) {
			minimum = min(timeSent[i], minimum);
		}
		first[iteration] = minimum;
	}

	// calculate averages
	int sumFirst = 0, sumSecond = 0, sumLast = 0;
	int i;
	for (i = 0; i < ITERATIONS; i++)
		sumFirst += first[i];
	
	printf("\nAverage first delay: %d\n", sumFirst/ITERATIONS);
}
