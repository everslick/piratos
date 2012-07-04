#include <stdio.h>

#include "shell.h"

void
cmd_info_help(void) {
	printf("'info' prints OS name and version.\n");
}

int
cmd_info_exec(char *argv[]) {
	printf("pir{A}tos Version " VERSION " (" PLATFORM ")\n");

	return (0);
}
