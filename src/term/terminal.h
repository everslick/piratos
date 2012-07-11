#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <sys/ioctl.h>
#include <termios.h>

#include <vector>

#include <kernel/hal/fb/fb.h>

#include "vt102.h"
#include "shell.h"

class Terminal {

public:

	Terminal(FB_Surface *f[]);
	~Terminal(void);

	bool Update(void);

	bool KeyEvent(int key, int code, int modifier, int state);

private:

	FB_Rectangle geometry;
	FB_Surface *surface;
	FB_Surface **font;

	Shell *CreateShell(int nr = -1);
	void SwitchShell(int nr);

	void DetermineColor(VT102::CanvasChar &c, int &fg, int &bg);

	void Repaint(void);

	std::vector<Shell *> shells;
	Shell *shell;

	char *program_to_start;

	int current_shell, virtual_terminals;

	int cw, ch, width, height;

};

#endif // _TERMINAL_H_
