#include "shell.h"

void
cmd_pwd_help(void) {
	shell_print("'pwd' prints the current working directory.\n");
}

int
cmd_pwd_exec(char *argv[]) {
	shell_print("%s\n", shell_cwd);

	return (0);
}
