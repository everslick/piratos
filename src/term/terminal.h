#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <kernel/hal/kbd/kbd.h>
#include <kernel/hal/fb/fb.h>

#include "shell.h"

#define NUM_VIRT_TERMS 8

typedef struct {
	FB_Rectangle geometry;
	FB_Surface *surface;
	FB_Surface *font[8];

	Shell *shells[NUM_VIRT_TERMS];

	int shell, cw, ch, width, height;
} Terminal;

Terminal *terminal_new(GFX_Bitmap *bitmap);
void      terminal_free(Terminal *terminal);

void terminal_update(void *arg);
void terminal_key_event(Terminal *terminal, KBD_Event *ev);

#endif // _TERMINAL_H_
