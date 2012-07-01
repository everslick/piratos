#include <unistd.h>
#include <time.h>

void piratos(void);

static struct timespec start;

void
vApplicationIdleHook( void ) {
	sleep(1);
}

int
main(int argc, char **argv) {
	clock_gettime(CLOCK_MONOTONIC, &start);
	//int now = (t.tv_sec-start.tv_sec) * 1000 + (t.tv_nsec-start.tv_nsec) / 1000000;

	piratos();

	/* Should never get here! */

	return (0);
}
