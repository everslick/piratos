#include "cli.h"

void
cmd_info_help(CLI *cli) {
	cli_print(cli, "'info' prints OS name and version.\n");
}

int
cmd_info_exec(CLI *cli, char *argv[]) {
	cli_print(cli, "pir{A}tos Version " VERSION " (" PLATFORM ")\n");

	return (0);
}
