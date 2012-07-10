#include "shell.h"

void
cmd_info_help(void) {
	shell_print("'info' prints OS name and version.\n");
}

int
cmd_info_exec(char *argv[]) {
	shell_print("pir{A}tos Version " VERSION " (" PLATFORM ")\n");

	return (0);
}
