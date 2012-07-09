#include "mouse.h"

#include "SDL.h"

int
mouse_init(void) {
	SDL_Init(SDL_INIT_VIDEO);

	return (0);
}

int
mouse_fini(void) {
	return (0);
}

int
mouse_poll(MOUSE_Event *ev) {
	SDL_Event sdl_ev;
	int mask, ret = 0;

	if (ev) {
		ev->type = MOUSE_EVENT_TYPE_NONE;
		ev->state = MOUSE_EVENT_STATE_NONE;
		ev->button = 0;
		ev->x = 0;
		ev->y = 0;
	}

	SDL_PumpEvents();

	mask = (SDL_EVENTMASK (SDL_MOUSEBUTTONDOWN) |
	        SDL_EVENTMASK (SDL_MOUSEBUTTONUP)   |
	        SDL_EVENTMASK (SDL_MOUSEMOTION));

	if (SDL_PeepEvents(&sdl_ev, 1, SDL_GETEVENT, mask) == 1) {
		if (ev) {
			switch (sdl_ev.type) {
				case SDL_MOUSEMOTION:
					ev->type = MOUSE_EVENT_TYPE_MOTION;

					ev->x = sdl_ev.motion.x;
					ev->y = sdl_ev.motion.y;
				break;

				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					ev->type = MOUSE_EVENT_TYPE_BUTTON;

					if (sdl_ev.button.state == SDL_PRESSED)  ev->state = MOUSE_EVENT_STATE_PRESSED;
					if (sdl_ev.button.state == SDL_RELEASED) ev->state = MOUSE_EVENT_STATE_RELEASED;

					ev->button = sdl_ev.button.button;
					ev->x = sdl_ev.button.x;
					ev->y = sdl_ev.button.y;
				break;
			}

			ret = (ev->state != MOUSE_EVENT_TYPE_NONE);
		}
	}

	return (ret);
}
