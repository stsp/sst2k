#include <stdlib.h>
#include <time.h>

void randomize(void) {
	srand((int)time(NULL));
}


int max(int a, int b) {
	if (a > b) return a;
	return b;
}

int min(int a, int b) {
	if (a < b) return a;
	return b;
}

