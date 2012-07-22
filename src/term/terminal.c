#include <stdlib.h>

#include <lib/gfx/glyph.h>
#include <srv/dpy/dpy.h>

#include "draw.h"

#include "terminal.h"

static void SwitchShell(Terminal *terminal, int nr);
static void DetermineColor(Terminal *terminal, struct VT102_CanvasChar *c, int *fg, int *bg);
static void Repaint(Terminal *terminal);

Terminal *
terminal_new(GFX_Bitmap *bitmap) {
	Terminal *terminal = (Terminal *)malloc(sizeof (Terminal));

	terminal->width  = 80;
	terminal->height = 32;

	// create 8 fonts for 8 ansi colors
	DrawFont(bitmap, terminal->font);

	// fonts MUST have been loaded before!
	terminal->cw = (*terminal->font)->width / 95;
	terminal->ch = (*terminal->font)->height;

	terminal->geometry.x = 10;
	terminal->geometry.y = 10;
	terminal->geometry.w = terminal->cw * terminal->width;
	terminal->geometry.h = terminal->ch * terminal->height;

	// create window surface
	terminal->surface = fb_create_surface(terminal->geometry.w, terminal->geometry.h, FB_FORMAT_BEST);

	// create virtual shells
	for (int i=0; i<NUM_VIRT_TERMS; i++) {
		terminal->shells[i] = shell_new();
	}

	// activate first virtual shell
	SwitchShell(terminal, 0);

	xTaskCreate(terminal_update, "Terminal", configMINIMAL_STACK_SIZE, terminal, 12, NULL);

	return (terminal);
}

void
terminal_free(Terminal *terminal) {
	// delete virtual shells
	for (int i=0; i<NUM_VIRT_TERMS; i++) {
		shell_free(terminal->shells[i]);
	}

	// free widget surface
	if (terminal->surface) fb_release_surface(terminal->surface);
}

static void
DetermineColor(Terminal *terminal, struct VT102_CanvasChar *c, int *fg, int *bg) {
	int color[2] = { 7, 0 }; // init fg = white & bg = black
	enum { FG, BG };

	// overwrite character specific colors
	if (c->use_fg) color[FG] = c->foreground;
	if (c->use_bg) color[BG] = c->background;

	// prevent invisible colors
	if ((color[FG] == color[BG]) && (c->foreground != c->background)) {
		if (color[FG] != 0) color[BG] = 0; else color[BG] = 7;
	}

	// swich colors when inverted
	if (c->inverted) {
		int col = color[FG];

		color[FG] = color[BG];
		color[BG] = col;
	}

	//  hide concealed cells
	if (c->concealed) color[FG] = color[BG];

	*fg = color[FG];
	*bg = color[BG];
}

static void
SwitchShell(Terminal *terminal, int nr) {
	// clamp shell number
	if (nr < 0) nr = 0;
	if (nr > NUM_VIRT_TERMS - 1) nr = NUM_VIRT_TERMS - 1;

	terminal->shell = nr;

	// it can be, that the geometry has changed
	VT102 *vt102 = terminal->shells[terminal->shell]->vt102;
	vt102_set_size(vt102, terminal->width, terminal->height);

	Repaint(terminal);
}

static void
Repaint(Terminal *terminal) {
	int fg, bg;

	VT102 *vt102 = terminal->shells[terminal->shell]->vt102;

	int cw = terminal->cw;
	int ch = terminal->ch;

	// draw vt102 canvas
	for (int y=0; y<terminal->height; y++) {
		struct VT102_CanvasChar *row = vt102_canvas(vt102)[y];

		for (int x=0; x<terminal->width; x++) {
			struct VT102_CanvasChar *cell = &row[x];

			DetermineColor(terminal, cell, &fg, &bg);

			// print char
			DrawGlyph(terminal->surface, terminal->font, x*cw, y*ch, cell->ch, fg, bg);

			// double bold characters
			if (cell->bold) {
				DrawGlyph(terminal->surface, terminal->font, x*cw+1, y*ch, cell->ch, fg, -1);
			}

			// draw underlined
			if (cell->underscore) {
				DrawLine(terminal->surface, x*cw, y*ch+ch-1, x*cw+cw-1, y*ch+ch-1, fg);
				DrawLine(terminal->surface, x*cw, y*ch+ch-2, x*cw+cw-1, y*ch+ch-2, fg);
			}
		}
	}

	// draw cursor
	if (vt102_cursor_visible(vt102)) {
		int x = vt102_cursor_x(vt102), y = vt102_cursor_y(vt102);

		if (y < terminal->height) {
			// the cursor can't get out of the screen
			if (x >= terminal->width) x = terminal->width - 1;

			struct VT102_CanvasChar *cell = &vt102_canvas(vt102)[y][x];

			DetermineColor(terminal, cell, &fg, &bg);

			// use inverse colors
			DrawGlyph(terminal->surface, terminal->font, x*cw, y*ch, cell->ch, bg, fg);
		}
	}

	// blit window to screen
	dpy_blit(terminal->surface, 0, &terminal->geometry);
	dpy_flip(&terminal->geometry);

	vt102_refreshed(vt102);
}

void
terminal_update(void *arg) {
	Terminal *terminal = (Terminal *)arg;
	VT102 *vt102;

	while (1) {
		vt102 = terminal->shells[terminal->shell]->vt102;

		if (vt102_to_refresh(vt102)) {
			Repaint(terminal);
		}

		if (vt102_to_ring(vt102)) vt102_bell_seen(vt102);

		vTaskDelay(10);
	}
}

void
terminal_key_event(Terminal *terminal, KBD_Event *ev) {
	int key_press_handled = 0;

	int key      = ev->symbol;
	//int code     = ev->unicode;
	int modifier = ev->modifier;
	int state    = ev->state;

	if (state != KBD_EVENT_STATE_PRESSED) return;

	if (modifier & KBD_MOD_LALT) {
		switch (key) {
			case KBD_KEY_LEFT:  SwitchShell(terminal, terminal->shell - 1); key_press_handled = 1; break;
			case KBD_KEY_RIGHT: SwitchShell(terminal, terminal->shell + 1); key_press_handled = 1; break;
		}
	}

	if (!key_press_handled) {
		shell_key_event(terminal->shells[terminal->shell], ev);
	}
}
