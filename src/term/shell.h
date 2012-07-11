#ifndef _WIDGET_TERMINAL_SHELL_H_
#define _WIDGET_TERMINAL_SHELL_H_

#include <sys/ioctl.h>
#include <termios.h>
#include <stdarg.h>

#include "vt102.h"

#define INPUT_BUFSIZE 1024

class Shell {

public:

	Shell(const char *ttl, const char *cmd, char * const argv[]);
	~Shell(void);

	void SetTerminalSize(int char_w, int char_h, int pixel_w, int pixel_h);

	void Write(const unsigned char *data, int len);

	bool HandleOutput(void);

	const char *GetTitle(void) { return (title);   }
	bool Running(void)         { return (running); }
	VT102 &GetScreen(void)     { return (vt102);   }

private:

	bool Open(const char *command, char * const argv[]);
	bool Close(void);

	void CloseAll(int fd);

	void CleanUp(void);

	bool ShellWaitPid(int pid, int timeout);

	int DebugOut(const char *fmt, ...);

	unsigned char buffer[INPUT_BUFSIZE];

	int child_pid, master_pty;
	bool running;

	char *title;

	VT102 vt102;

};

#endif // _WIDGET_TERMINAL_SHELL_H_
