#include <stdlib.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/termio.h>

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

int getch(void) {
	char chbuf[1];
	struct termio oldstate, newstate;
    ioctl(0,TCGETA,&oldstate);
	newstate = oldstate;
	newstate.c_iflag = 0;
	newstate.c_lflag = 0;
	ioctl(0,TCSETA,&newstate);
	read(0, &chbuf, 1);
    ioctl(0,TCSETA,&oldstate);
}