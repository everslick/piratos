#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <kernel/hal/kbd/kbd.h>
#include <kernel/hal/fb/fb.h>

#include "shell.h"

#define NUM_VIRT_TERMS 2

class Terminal {

public:

	Terminal(GFX_Bitmap *bitmap);
	~Terminal(void);

	bool Update(void);

	bool KeyEvent(KBD_Event *ev);

private:

	FB_Rectangle geometry;
	FB_Surface *surface;
	FB_Surface *font[8];

	void SwitchShell(int nr);

	void DetermineColor(VT102::CanvasChar &c, int &fg, int &bg);

	void Repaint(void);

	Shell *shells[NUM_VIRT_TERMS];

	int shell;

	int cw, ch, width, height;

};

#endif // _TERMINAL_H_
