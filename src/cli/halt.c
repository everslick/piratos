#include <stdlib.h>

#include "cli.h"

void
cmd_halt_help(CLI *cli) {
	printf("'halt' terminates the OS.\n");
}

int
cmd_halt_exec(CLI *cli, char *argv[]) {
	exit(0);

	return (0);
}
