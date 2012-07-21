#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "shell.h"

Shell *
shell_new(void) {
	Shell *shell = (Shell *)malloc(sizeof (Shell));

	shell->cmd = NULL;

	shell->vt102 = vt102_new();

	shell->cli = cli_new(shell->vt102);

	shell->input = input_new(shell->vt102);

	xTaskCreate(shell_main, "SHELLx",  configMINIMAL_STACK_SIZE, shell, 8, &shell->task);

	return (shell);
}

void
shell_free(Shell *shell) {
	vTaskDelete(shell->task);

	input_free(shell->input);

	cli_free(shell->cli);

	vt102_free(shell->vt102);
}

void
shell_main(void *thiz) {
	Shell *s = (Shell *)thiz;
	char *args[MAX_ARGUMENT_NUM];
	int nargs;

	cli_set_cwd(s->cli, (char *)"/");
	cli_set_prompt(s->cli, (char *)"/");

	while (!s->cli->cli_quit) {
		input_new_line(s->input, s->cli->cli_prompt);

		while (!s->cmd) {
			vTaskDelay(20);
		}		

		cli_parse_args(s->cmd, args, &nargs);
		s->cmd = NULL;

		if (nargs == 0) continue;

		// special case for man command
		if (!strcmp(args[0], "man")) {
			if (nargs == 2) {
				if (cli_cmd_help(s->cli, args) != 0) {
					cli_print(s->cli, "man: no manual for command '%s' available\n", args[1]);
				}
			} else {
				cli_print(s->cli, "USAGE: man [command]\n");
			}

			continue;
		}

		if (cli_cmd_exec(s->cli, args) == 0) continue;

		cli_print(s->cli, "%s: command not found\n", args[0]);
	}
}

int
shell_key_event(Shell *shell, KBD_Event *ev) {
	static const char *left   = "\033[D";
	static const char *right  = "\033[C";
	static const char *up     = "\033[A";
	static const char *down   = "\033[B";
	static const char *pgup   = "\033[5~";
	static const char *pgdown = "\033[6~";
	static const char *home   = "\033OH";
	static const char *end    = "\033OF";
	static const char *insert = "\033[2~";
	static const char *del    = "\033[3~";

	int key      = ev->symbol;
	int code     = ev->unicode;
	int modifier = ev->modifier;
	int state    = ev->state;

	if (state != KBD_EVENT_STATE_PRESSED) return (0);

	if (modifier & KBD_MOD_CTRL) {
		switch (key) {
			case KBD_KEY_d: /* logout shell */ break;
		}
	} else {
		switch (key) {
			case KBD_KEY_LEFT:     vt102_puts(shell->vt102, left,   3); break;
			case KBD_KEY_RIGHT:    vt102_puts(shell->vt102, right,  3); break;
			case KBD_KEY_UP:       vt102_puts(shell->vt102, up,     3); break;
			case KBD_KEY_DOWN:     vt102_puts(shell->vt102, down,   3); break;
			case KBD_KEY_PAGEUP:   vt102_puts(shell->vt102, pgup,   4); break;
			case KBD_KEY_PAGEDOWN: vt102_puts(shell->vt102, pgdown, 4); break;
			case KBD_KEY_HOME:     vt102_puts(shell->vt102, home,   3); break;
			case KBD_KEY_END:      vt102_puts(shell->vt102, end,    3); break;
			case KBD_KEY_INSERT:   vt102_puts(shell->vt102, insert, 4); break;
			case KBD_KEY_DELETE:   vt102_puts(shell->vt102, del,    4); break;

			default:
				if ((key <= KBD_KEY_DELETE) && (key > KBD_KEY_FIRST)) {
					vt102_puts(shell->vt102, (char *)&code, 1);
				}
			break;
		}
	}

	input_key_event(shell->input, ev);

	if (key == KBD_KEY_RETURN) {
		input_read_line(shell->input, &shell->cmd);

		vt102_putc(shell->vt102, CR);
		vt102_putc(shell->vt102, LF);
	}

	return (1);
}
