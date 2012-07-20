#include <stdlib.h>

#include <lib/gfx/glyph.h>

#include "draw.h"

#include "terminal.h"

static void SwitchShell(Terminal *terminal, int nr);
static void DetermineColor(Terminal *terminal, VT102::CanvasChar &c, int &fg, int &bg);
static void Repaint(Terminal *terminal);

Terminal *
terminal_new(GFX_Bitmap *bitmap) {
	FB_Rectangle r;

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

	r.x = terminal->geometry.x - 2;
	r.y = terminal->geometry.y - 2;
	r.w = terminal->geometry.w + 4;
	r.h = terminal->geometry.h + 4;

	// draw white frame
	FB_Color bg = { 0x80, 0xff, 0x80, 0xff  };
	fb_fill(0, &r, &bg);

	// create virtual shells
	for (int i=0; i<NUM_VIRT_TERMS; i++) {
		terminal->shells[i] = shell_new();
	}

	// activate first virtual shell
	SwitchShell(terminal, 0);

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
DetermineColor(Terminal *terminal, VT102::CanvasChar &c, int &fg, int &bg) {
	int color[2] = { 7, 0 }; // init fg = white & bg = black
	enum { FG, BG };

	// overwrite character specific colors
	if (c.use_fg) color[FG] = c.foreground;
	if (c.use_bg) color[BG] = c.background;

	// prevent invisible colors
	if ((color[FG] == color[BG]) && (c.foreground != c.background)) {
		if (color[FG] != 0) color[BG] = 0; else color[BG] = 7;
	}

	// swich colors when inverted
	if (c.inverted) {
		int col = color[FG];

		color[FG] = color[BG];
		color[BG] = col;
	}

	//  hide concealed cells
	if (c.concealed) color[FG] = color[BG];

	fg = color[FG];
	bg = color[BG];
}

static void
SwitchShell(Terminal *terminal, int nr) {
	// clamp shell number
	if (nr < 0) nr = 0;
	if (nr > NUM_VIRT_TERMS - 1) nr = NUM_VIRT_TERMS - 1;

	terminal->shell = nr;

	// it can be, that the geometry has changed
	terminal->shells[terminal->shell]->vt102->SetSize(terminal->width, terminal->height);

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
		const VT102::CanvasChar *row = vt102->Canvas()[y];

		for (int x=0; x<terminal->width; x++) {
			VT102::CanvasChar &cell = (VT102::CanvasChar &)row[x];

			DetermineColor(terminal, cell, fg, bg);

			// print char
			DrawGlyph(terminal->surface, terminal->font, x*cw, y*ch, cell.ch, fg, bg);

			// double bold characters
			if (cell.bold) {
				DrawGlyph(terminal->surface, terminal->font, x*cw+1, y*ch, cell.ch, fg, -1);
			}

			// draw underlined
			if (cell.underscore) {
				DrawLine(terminal->surface, x*cw, y*ch+ch-1, x*cw+cw-1, y*ch+ch-1, fg);
				DrawLine(terminal->surface, x*cw, y*ch+ch-2, x*cw+cw-1, y*ch+ch-2, fg);
			}
		}
	}

	// draw cursor
	if (vt102->CursorVisible()) {
		int x = vt102->CursorX(), y = vt102->CursorY();

		if (y < terminal->height) {
			// the cursor can't get out of the screen
			if (x >= terminal->width) x = terminal->width - 1;

			VT102::CanvasChar &cell =
				(VT102::CanvasChar &)vt102->Canvas()[y][x];

			DetermineColor(terminal, cell, fg, bg);

			// use inverse colors
			DrawGlyph(terminal->surface, terminal->font, x*cw, y*ch, cell.ch, bg, fg);
		}
	}

	// blit window to screen
	fb_blit(terminal->surface, 0, 0, &terminal->geometry, 0);
	fb_flip(&terminal->geometry);

	vt102->Refreshed();
}

int
terminal_update(Terminal *terminal) {
	int ret = false;

	VT102 *vt102 = terminal->shells[terminal->shell]->vt102;

	//shell->HandleOutput();

	if (vt102->ToRefresh()) {
		ret = true;
		Repaint(terminal);
	}

	if (vt102->ToRing()) vt102->BellSeen();

	return (ret);
}

int
terminal_key_event(Terminal *terminal, KBD_Event *ev) {
	int key_press_handled = 0;

	int key      = ev->symbol;
	//int code     = ev->unicode;
	int modifier = ev->modifier;
	int state    = ev->state;

	if (state != KBD_EVENT_STATE_PRESSED) return (true);

	if (modifier & KBD_MOD_LALT) {
		switch (key) {
			case KBD_KEY_LEFT:  SwitchShell(terminal, terminal->shell - 1); key_press_handled = 1; break;
			case KBD_KEY_RIGHT: SwitchShell(terminal, terminal->shell + 1); key_press_handled = 1; break;
		}
	}

	if (!key_press_handled) {
		shell_key_event(terminal->shells[terminal->shell], ev);
	}

	return (true);
}
