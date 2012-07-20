#include "cli.h"

void
cmd_pwd_help(CLI *cli) {
	cli_print(cli, "'pwd' prints the current working directory.\n");
}

int
cmd_pwd_exec(CLI *cli, char *argv[]) {
	cli_print(cli, "%s\n", cli->cli_cwd);

	return (0);
}
