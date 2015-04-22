#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
	int k;
	k = min(n, 10);
	k = (int)pow(2, k);
	return rand() % k;
}

int done(int *stations) {
	int i;
	for (i = 0; i < 5; i++) {
		if (!stations[i]) {
			return 0;
		}
	}
	return 1;
}

void main() {
	srand(time(NULL));
	int first[500], second[500], fifth[500];
	int iteration;
	for (iteration = 0; iteration < 500; iteration++) {

		// init data for iteration
		int stations[5] = {0};
		int backoff[5] = {0};
		int collisions[5] = {0};
		int sending[5] = {0};
		int T = 1;

		// loop for all stations to transmit
		while (!done(stations)) {

			// set transmitted flag to false
			int transmitted = 0;

			int i;
			// attempt to transmit
			for (i = 0; i < 5; i++) {

				// make sure the station needs to transmit
				if (!stations[i] && !backoff[i]) {
					sending[i] = 1;
					// check if another station is transmitting in this timeslot
					if (!transmitted) {
						transmitted = 1;
						stations[i] = T;
					}
					else {
						int j;

						// set backoff for all stations transmitting this timeslot
						for (j = 0; j < 5; j++) {
							if (sending[j]) {
								collisions[j]++;
								backoff[j] = computeBackoff(collisions[j]);
								stations[j] = 0;
							}
						}
					}
				}
				// code for station that is backing off
				else if (!stations[i] && backoff[i]) {
					backoff[i]--;
				}
			}

			// increment time
			T++;

			// reset sending flags
			for (i = 0; i < 5; i++)
				sending[i] = 0;
		}

		int minimum = 500;
		int i;

		// calculate delay of first transmitted station
		for (i = 0; i < 5; i++) {
			minimum = min(stations[i], minimum);
		}
		first[iteration] = minimum;
		minimum = 500;

		// calculate delay of second transmitted station
		for (i = 0; i < 5; i++) {
			if (stations[i] > first[iteration]) {
				minimum = min(stations[i], minimum);
			}
		}
		second[iteration] = minimum;

		int maximum = 0;
		// calculate delay of fifth transmitted station
		for (i = 0; i < 5; i++) {
			maximum = max(stations[i], maximum);
		}
		fifth[iteration] = maximum;



		// print all
		for (i = 0; i < 5; i++)
			printf("1: %2d   2: %2d   3: %2d   4: %2d   5: %2d\n", stations[0], stations[1], stations[2], stations[3], stations[4]);

	}

	// calculate averages
	int sumFirst = 0, sumSecond = 0, sumFifth = 0;
	int i;
	for (i = 0; i < 500; i++) {
		sumFirst += first[i];
		sumSecond += second[i];
		sumFifth += fifth[i];
	}
	printf("\nAverage first delay: %d\nAverage second delay: %d\nAverage fifth delay: %d\n", sumFirst/500 - 1, sumSecond/500 - 1, sumFifth/500 - 1);
}
