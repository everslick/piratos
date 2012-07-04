#include <stdio.h>

#include "fat_filelib.h"

#include "shell.h"

void
cmd_ls_help(void) {
	printf("'ls' lists files and directories.                          \n");
	printf("                                                           \n");
	printf("If no path is given the current working directory is used. \n");
}

int
cmd_ls_exec(char *argv[]) {
	int argc = shell_num_args(argv);
	char path[PATH_MAX];

	if (argc > 2) {
		printf("Too many arguments to 'ls'\n");

		return (-1);
	}

	if (argc < 2) {
		shell_clean_path("", path); // cwd
	} else {
		shell_clean_path(argv[1], path);
	}

	if (!fl_is_dir(path)) {
		printf("Invalid directory '%s'\n", argv[1]);

		return (-1);
	}

	fl_listdirectory(path);

	return (0);
}
