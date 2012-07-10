#include "shell.h"

void
cmd_mv_help(unsigned int level) {
	shell_print("'mv' renames files\n");
}

int
cmd_mv_exec(char *argv[]) {
	char from[PATH_MAX], to[PATH_MAX];
	int argc = shell_num_args(argv);

	if (argc != 3) {
		shell_print("mv: invalid number of arguments.\n");

		return (-1);
	}

	shell_clean_path(argv[1], from);
	shell_clean_path(argv[2], to);

	if (1/*rename(from, to)*/) {
		shell_print("Unable to rename %s to %s\n", from, to);

		return (-1);
	}

	return (0);
}
