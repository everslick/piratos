#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stropts.h>

#include <lib/gfx/glyph.h>

#include <kernel/hal/kbd/kbd.h>

#include "draw.h"

#include "terminal.h"

using namespace std;

Terminal::Terminal(FB_Surface *f[]) {
	program_to_start = (char *)"/bin/bash";
	virtual_terminals = 8;
	FB_Rectangle r;

	font = f;

	surface = NULL;
	shell   = NULL;

	width = 80;
	height = 32;

	cw = font[0]->width / 95;
	ch = font[0]->height;

	geometry.x = 10;
	geometry.y = 10;
	geometry.w = cw * width;
	geometry.h = ch * height;

	r.x = geometry.x - 2;
	r.y = geometry.x - 2;
	r.w = geometry.w + 4;
	r.h = geometry.h + 4;

	// create widget surface
	surface = fb_create_surface(geometry.w, geometry.h, FB_FORMAT_BEST);

	// draw white frame
	FB_Color bg = { 0xff, 0xff, 0xff, 0xff  };
	fb_fill(NULL, &r, &bg);

	for (int i=0; i<virtual_terminals; i++) {
		CreateShell();
	}

	// activate first virtual shell
	SwitchShell(0);
}

Terminal::~Terminal(void) {
	vector<Shell *>::iterator it;

	// delete virtual shells
	for (it=shells.begin(); it!=shells.end(); it++) {
		delete (*it);
	}

	// free widget surface
	if (surface) fb_release_surface(surface);
}

Shell *
Terminal::CreateShell(int nr) {
	char * const args[] = { program_to_start, NULL };

	Shell *p = new Shell("Shell", program_to_start, args);

	if (nr != -1) {
		if (p && (nr < (int)shells.size())) shells[nr] = p;
	} else {
		if (p) shells.push_back(p);
	}

	return (p);
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

	// now really set the color structs
	fg = color[FG];
	bg = color[BG];
}

void
Terminal::SwitchShell(int nr) {
	// clamp shell number
	if (nr < 0) nr = 0;
	if (nr >= (int)shells.size()) nr = (int)shells.size() - 1;

	// activate new shell
	if ((shell = shells[nr])) {
		// it can be, that the geometry has changed
		shell->SetTerminalSize(width, height, geometry.w, geometry.h);
	}

	current_shell = nr;

	Repaint();
}

void
Terminal::Repaint(void) {
	int fg, bg;

	if (!shell || !surface) return;

	VT102 &vt102 = shell->GetScreen();

	// draw vt102 canvas
	for (int y=0; y<height; y++) {
		const VT102::CanvasChar *row = vt102.Canvas()[y];

		for (int x=0; x<width; x++) {
			VT102::CanvasChar &cell = (VT102::CanvasChar &)row[x];

			DetermineColor(cell, fg, bg);

			// print char
			DrawGlyph(surface, x*cw, y*ch, cell.ch, font, fg, bg);

			// double bold characters
			if (cell.bold) {
				DrawGlyph(surface, x*cw+1, y*ch, cell.ch, font, fg, -1);
			}

			// draw underlined
			if (cell.underscore) {
				DrawLine(surface, x*cw, y*ch+ch-1, x*cw+cw-1, y*ch+ch-1, fg);
				DrawLine(surface, x*cw, y*ch+ch-2, x*cw+cw-1, y*ch+ch-2, fg);
			}
		}
	}

	// draw cursor
	if (vt102.CursorVisible()) {
		int cx = vt102.CursorX(), cy = vt102.CursorY();

		if (cy < height) { 
			// the cursor can't get out of the screen
			if (cx >= width) cx = width - 1;

			VT102::CanvasChar& cell =
				(VT102::CanvasChar &)vt102.Canvas()[cy][cx];

			DetermineColor(cell, fg, bg);

			// use inverse colors
			DrawGlyph(surface, cx*cw, cy*ch, cell.ch, font, bg, fg);
		}
	}

	// draw frame
	//FB_Color wht = { 0xff, 0xff, 0xff, 0xff };
	//DrawOutline(surface, NULL, &wht);

	fb_blit(surface, NULL, NULL, &geometry, NULL);
	fb_flip(&geometry);

	vt102.Refreshed();
}

bool
Terminal::Update(void) {
	bool ret = false;

	if (!shell) return (false);

	VT102 &vt102 = shell->GetScreen();

	if (!shell->HandleOutput()) {
		// child process has been terminated
		delete (shell);
		shell = CreateShell(current_shell);
	}

	if (vt102.ToRefresh()) {
		ret = true;
		Repaint();
	}

	if (vt102.ToRing()) vt102.BellSeen();

	return (ret);
}

bool
Terminal::KeyEvent(int key, int code, int modifier, int state) {
	static const unsigned char *left   = (unsigned char *)"\033[D";
	static const unsigned char *right  = (unsigned char *)"\033[C";
	static const unsigned char *up     = (unsigned char *)"\033[A";
	static const unsigned char *down   = (unsigned char *)"\033[B";
	static const unsigned char *pgup   = (unsigned char *)"\033[5~";
	static const unsigned char *pgdown = (unsigned char *)"\033[6~";
	static const unsigned char *home   = (unsigned char *)"\033OH";
	static const unsigned char *end    = (unsigned char *)"\033OF";
	static const unsigned char *insert = (unsigned char *)"\033[2~";
	static const unsigned char *del    = (unsigned char *)"\033[3~";

	if (!shell) return (false);

	if (state != KBD_EVENT_STATE_PRESSED) return (true);

	if (modifier == KBD_MOD_LALT) {
		switch (key) {
			case KBD_KEY_LEFT:  SwitchShell(current_shell - 1); break;
			case KBD_KEY_RIGHT: SwitchShell(current_shell + 1); break;
		}
	} else {
		switch (key) {
			case KBD_KEY_LEFT:     shell->Write(left,   3); break;
			case KBD_KEY_RIGHT:    shell->Write(right,  3); break;
			case KBD_KEY_UP:       shell->Write(up,     3); break;
			case KBD_KEY_DOWN:     shell->Write(down,   3); break;
			case KBD_KEY_PAGEUP:   shell->Write(pgup,   4); break;
			case KBD_KEY_PAGEDOWN: shell->Write(pgdown, 4); break;
			case KBD_KEY_HOME:     shell->Write(home,   3); break;
			case KBD_KEY_END:      shell->Write(end,    3); break;
			case KBD_KEY_INSERT:   shell->Write(insert, 4); break;
			case KBD_KEY_DELETE:   shell->Write(del,    4); break;

			default:
				if ((key <= KBD_KEY_DELETE) && (key > KBD_KEY_FIRST)) {
					shell->Write((unsigned char *)&code, 1);
				}
			break;
		}
	}

	return (true);
}
