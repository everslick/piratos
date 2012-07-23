#include "cli.h"

void
cmd_info_help(CLI *cli) {
	printf("'info' prints OS name and version.\n");
}

int
cmd_info_exec(CLI *cli, char *argv[]) {
	printf("pir{A}tos Version " VERSION " (" PLATFORM ")\n");

	return (0);
}
