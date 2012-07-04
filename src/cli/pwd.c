#include <stdio.h>

#include "shell.h"

void
cmd_pwd_help(void) {
	printf("'pwd' prints the current working directory.\n");
}

int
cmd_pwd_exec(char *argv[]) {
	printf("%s\n", shell_cwd);

	return (0);
}
