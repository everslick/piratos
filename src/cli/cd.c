#include <string.h>

#include "fat_filelib.h"

#include "shell.h"

void
cmd_cd_help(void) {
	shell_print("'cd' changes the current working directory.\n");
}

int
cmd_cd_exec(char *argv[]) {
	int argc = shell_num_args(argv);
	char path[PATH_MAX];

	if (argc > 2) {
		shell_print("Too many arguments to 'cd'\n");

		return (-1);
	}

	if (argc < 2) {
		shell_print("No directory specified. Try 'man cd'\n");

		return (-1);
	}

	if (!strcmp(argv[1], "-")) {
		shell_get_owd(path);
	} else {
		shell_clean_path(argv[1], path);
	}

	if (!fl_is_dir(path)) {
		shell_print("Invalid directory '%s'\n", path);

		return (-1);
	}

	shell_set_cwd(path);
	shell_set_prompt(path);

	return (0);
}
