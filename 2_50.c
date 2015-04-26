#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ITERATIONS 100
#define STATIONS 5

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

int done(int *stations) {
	int i;
	for (i = 0; i < STATIONS; i++) {
		if (!stations[i]) {
			return 0;
		}
	}
	return 1;
}

void main() {
	srand(time(NULL));
	int first[ITERATIONS], second[ITERATIONS], last[ITERATIONS], firstIsTwoCount = 0, firstOverTenCount = 0;
	int iteration;
	for (iteration = 0; iteration < ITERATIONS; iteration++) {

		// init data for iteration
		int timeSent[STATIONS] = {0};
		int nextTimeToSend[STATIONS] = {0};
		int collisionCount[STATIONS] = {0};
		int T = 0;

		// loop for all stations to transmit
		while (!done(timeSent)) {

			// set sending counter to 0, collision to false
			int sending = 0;
			int collision = 0;

			int sendingIndex = -1;

			int i;
			// check each station
			for (i = 0; i < STATIONS; i++) {
				int j;
				// check if this is the only stations trying to send
				for (j = 0; j < STATIONS; j++)
					if (nextTimeToSend[j] == T) {
						sending++;
						sendingIndex = j;
					}

				// attempt to transmit if not waiting
				if (sending == 1) {
					timeSent[sendingIndex] = T;
				}
				else if (sending > 1) {
					for (j = 0; j < STATIONS; j++)
						if (nextTimeToSend[j] == T) {
							nextTimeToSend[j] = T + computeBackoff(collisionCount[j]);
							collisionCount[j]++;
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
		first[iteration] = minimum - 1;
		if (minimum == 2)
			firstIsTwoCount++;
		else if (minimum > 10)
			firstOverTenCount++;
		minimum = 9999;

		// calculate delay of second transmitted station
		for (i = 0; i < STATIONS; i++) {
			if (timeSent[i] > first[iteration] + 1) {
				minimum = min(timeSent[i], minimum);
			}
		}
		second[iteration] = minimum - 1;

		int maximum = 0;
		// calculate delay of last transmitted station
		for (i = 0; i < STATIONS; i++) {
			maximum = max(timeSent[i], maximum);
		}
		last[iteration] = maximum - 1;



		// print all times transmitted
		printf("%d:", iteration+1);
		for (i = 0; i < STATIONS; i++) {
			if (i % 6 == 0)
				printf("\n%3d: %4d", i+1, timeSent[i]);
			else
				printf("   %3d: %4d", i+1, timeSent[i]);
		}
		printf("\n\n");

	}

	// calculate averages
	int sumFirst = 0, sumSecond = 0, sumLast = 0;
	int i;
	for (i = 0; i < ITERATIONS; i++) {
		sumFirst += first[i];
		sumSecond += second[i];
		sumLast += last[i];
	}
	printf("First is two %d times\nFirst over ten %d times\n", firstIsTwoCount, firstOverTenCount);
	printf("Sums:  first=%d, second=%d, last=%d\n", sumFirst, sumSecond, sumLast);
	printf("\nAverage first delay: %d\nAverage second delay: %d\nAverage last delay: %d\n", sumFirst/ITERATIONS, sumSecond/ITERATIONS, sumLast/ITERATIONS);
}
