#include <unistd.h>
#include <time.h>

#include <SDL.h>

#include <srv/dpy/dpy.h>

void piratos(void);

static struct timespec start;

void
vApplicationIdleHook( void ) {
	dpy_update();

	sleep(1);
}

int
main(int argc, char **argv) {
	clock_gettime(CLOCK_MONOTONIC, &start);
	//int now = (t.tv_sec-start.tv_sec) * 1000 + (t.tv_nsec-start.tv_nsec) / 1000000;

	SDL_Init(SDL_INIT_NOPARACHUTE);

	piratos();

	/* Should never get here! */

	SDL_Quit();

	return (0);
}
