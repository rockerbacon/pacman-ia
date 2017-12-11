#include "random.h"
#include <time.h>
#include <stdlib.h>

void lab309::timeSeed (void) {
	srand(time(NULL));
}

int lab309::randomBetween (int lower, int upper) {
	return lower + rand() % (upper - lower);
}

float lab309::random (void) {
	return (float)(rand())/RAND_MAX;
}
