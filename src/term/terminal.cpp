#include <lib/gfx/glyph.h>

#include "draw.h"

#include "terminal.h"

using namespace std;

Terminal::Terminal(GFX_Bitmap *bitmap) {
	FB_Rectangle r;

	width  = 80;
	height = 32;

	// create 8 fonts for 8 ansi colors
	DrawFont(bitmap, font);

	// fonts MUST have been loaded before!
	cw = (*font)->width / 95;
	ch = (*font)->height;

	geometry.x = 10;
	geometry.y = 10;
	geometry.w = cw * width;
	geometry.h = ch * height;

	// create window surface
	surface = fb_create_surface(geometry.w, geometry.h, FB_FORMAT_BEST);

	r.x = geometry.x - 2;
	r.y = geometry.y - 2;
	r.w = geometry.w + 4;
	r.h = geometry.h + 4;

	// draw white frame
	FB_Color bg = { 0x80, 0xff, 0x80, 0xff  };
	fb_fill(0, &r, &bg);

	// create virtual shells
	for (int i=0; i<NUM_VIRT_TERMS; i++) {
		shells[i] = new Shell();
	}

	// activate first virtual shell
	SwitchShell(0);
}

Terminal::~Terminal(void) {
	// delete virtual shells
	for (int i=0; i<NUM_VIRT_TERMS; i++) {
		delete (shells[i]);
	}

	// free widget surface
	if (surface) fb_release_surface(surface);
}

void
Terminal::DetermineColor(VT102::CanvasChar &c, int &fg, int &bg) {
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

void
Terminal::SwitchShell(int nr) {
	// clamp shell number
	if (nr < 0) nr = 0;
	if (nr > NUM_VIRT_TERMS - 1) nr = NUM_VIRT_TERMS - 1;

	shell = nr;

	// it can be, that the geometry has changed
	shells[shell]->vt102->SetSize(width, height);

	Repaint();
}

void
Terminal::Repaint(void) {
	int fg, bg;

	VT102 *vt102 = shells[shell]->vt102;

	// draw vt102 canvas
	for (int y=0; y<height; y++) {
		const VT102::CanvasChar *row = vt102->Canvas()[y];

		for (int x=0; x<width; x++) {
			VT102::CanvasChar &cell = (VT102::CanvasChar &)row[x];

			DetermineColor(cell, fg, bg);

			// print char
			DrawGlyph(surface, font, x*cw, y*ch, cell.ch, fg, bg);

			// double bold characters
			if (cell.bold) {
				DrawGlyph(surface, font, x*cw+1, y*ch, cell.ch, fg, -1);
			}

			// draw underlined
			if (cell.underscore) {
				DrawLine(surface, x*cw, y*ch+ch-1, x*cw+cw-1, y*ch+ch-1, fg);
				DrawLine(surface, x*cw, y*ch+ch-2, x*cw+cw-1, y*ch+ch-2, fg);
			}
		}
	}

	// draw cursor
	if (vt102->CursorVisible()) {
		int x = vt102->CursorX(), y = vt102->CursorY();

		if (y < height) {
			// the cursor can't get out of the screen
			if (x >= width) x = width - 1;

			VT102::CanvasChar &cell =
				(VT102::CanvasChar &)vt102->Canvas()[y][x];

			DetermineColor(cell, fg, bg);

			// use inverse colors
			DrawGlyph(surface, font, x*cw, y*ch, cell.ch, bg, fg);
		}
	}

	// blit window to screen
	fb_blit(surface, 0, 0, &geometry, 0);
	fb_flip(&geometry);

	vt102->Refreshed();
}

bool
Terminal::Update(void) {
	bool ret = false;

	VT102 *vt102 = shells[shell]->vt102;

	//shell->HandleOutput();

	if (vt102->ToRefresh()) {
		ret = true;
		Repaint();
	}

	if (vt102->ToRing()) vt102->BellSeen();

	return (ret);
}

bool
Terminal::KeyEvent(KBD_Event *ev) {
	int key_press_handled = 0;

	int key      = ev->symbol;
	int code     = ev->unicode;
	int modifier = ev->modifier;
	int state    = ev->state;

	if (state != KBD_EVENT_STATE_PRESSED) return (true);

	if (modifier & KBD_MOD_LALT) {
		switch (key) {
			case KBD_KEY_LEFT:  SwitchShell(shell - 1); key_press_handled = 1; break;
			case KBD_KEY_RIGHT: SwitchShell(shell + 1); key_press_handled = 1; break;
		}
	}

	if (!key_press_handled) {
		shells[shell]->KeyEvent(ev);
	}

	return (true);
}
