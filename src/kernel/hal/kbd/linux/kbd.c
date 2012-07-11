#include "kbd.h"

#include "SDL.h"

int
kbd_init(void) {
	SDL_Init(SDL_INIT_VIDEO);

	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	return (0);
}

int
kbd_fini(void) {
	return (0);
}

int
kbd_poll(KBD_Event *ev) {
	int mask = 0, ret = 0;
	SDL_Event sdl_ev;

	if (ev) {
		ev->type     = KBD_EVENT_TYPE_NONE;
		ev->state    = KBD_EVENT_STATE_NONE;
		ev->symbol   = 0;
		ev->unicode  = 0;
		ev->modifier = 0;
	}

	SDL_PumpEvents();

	mask = (SDL_EVENTMASK(SDL_KEYDOWN) | SDL_EVENTMASK(SDL_KEYUP));

	if (SDL_PeepEvents(&sdl_ev, 1, SDL_GETEVENT, mask) == 1) {
		if (ev) {
			ev->type = KBD_EVENT_TYPE_KEY;

			if (sdl_ev.key.state == SDL_PRESSED)  ev->state = KBD_EVENT_STATE_PRESSED;
			if (sdl_ev.key.state == SDL_RELEASED) ev->state = KBD_EVENT_STATE_RELEASED;

			ev->symbol   = sdl_ev.key.keysym.sym;
			ev->unicode  = sdl_ev.key.keysym.unicode;
			ev->modifier = sdl_ev.key.keysym.mod;

			ret = (ev->state != KBD_EVENT_TYPE_NONE);
		}
	}

	return (ret);
}
