#include <stdio.h>

#include "shell.h"

void
cmd_mv_help(unsigned int level) {
	printf("'mv' renames files\n");
}

int
cmd_mv_exec(char *argv[]) {
	char from[PATH_MAX], to[PATH_MAX];
	int argc = shell_num_args(argv);

	if (argc != 3) {
		printf("mv: invalid number of arguments.\n");

		return (-1);
	}

	shell_clean_path(argv[1], from);
	shell_clean_path(argv[2], to);

	if (rename(from, to)) {
		printf("Unable to rename %s to %s\n", from, to);

		return (-1);
	}

	return (0);
}
