#ifndef _SHELL_H_
#define _SHELL_H_

#include <kernel/os/task.h>

#include <kernel/hal/kbd/kbd.h>

#include "cli.h"
#include "vt102.h"
#include "input.h"


typedef struct {
	char *cmd;
	CLI *cli;

	Input *input;
	VT102 *vt102;

	xTaskHandle task;
} Shell;

Shell *shell_new(void);
void   shell_free(Shell *shell);

int shell_key_event(Shell *shell, KBD_Event *ev);

void shell_main(void *thiz);

#endif /* _SHELL_H_ */
