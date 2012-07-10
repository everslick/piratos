#include "shell.h"

void
cmd_quit_help(void) {
	shell_print("'quit' terminates the shell.\n");
}

int
cmd_quit_exec(char *argv[]) {
	shell_quit = 1;

	return (0);
}
