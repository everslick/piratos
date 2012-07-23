#include "cli.h"

void
cmd_pwd_help(CLI *cli) {
	printf("'pwd' prints the current working directory.\n");
}

int
cmd_pwd_exec(CLI *cli, char *argv[]) {
	printf("%s\n", cli->cli_cwd);

	return (0);
}
