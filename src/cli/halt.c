#include <stdio.h>
#include <stdlib.h>

#include "shell.h"

void
cmd_halt_help(void) {
	printf("'halt' terminates the OS.\n");
}

int
cmd_halt_exec(char *argv[]) {
	exit(0);

	return (0);
}
