#ifndef _SHELL_H_
#define _SHELL_H_

#include <kernel/os/task.h>

#include <kernel/hal/kbd/kbd.h>

#include "cli.h"
#include "vt102.h"
#include "input.h"

class Shell {

public:

	Shell(void);
	~Shell(void);

	bool KeyEvent(KBD_Event *ev);

	char *cmd;

	CLI *cli;

	Input *input;
	VT102 *vt102;

	xTaskHandle task;

	static void Main(void *thiz);

};

#endif /* _SHELL_H_ */
