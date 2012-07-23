#include "cli.h"

void
cmd_quit_help(CLI *cli) {
	printf("'quit' terminates the shell.\n");
}

int
cmd_quit_exec(CLI *cli, char *argv[]) {
	cli->cli_quit = 1;

	return (0);
}
