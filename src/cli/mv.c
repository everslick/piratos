#include "cli.h"

void
cmd_mv_help(CLI *cli) {
	printf("'mv' renames files\n");
}

int
cmd_mv_exec(CLI *cli, char *argv[]) {
	char from[PATH_MAX], to[PATH_MAX];
	int argc = cli_num_args(argv);

	if (argc != 3) {
		printf("mv: invalid number of arguments.\n");

		return (-1);
	}

	cli_clean_path(cli, argv[1], from);
	cli_clean_path(cli, argv[2], to);

	if (1/*rename(from, to)*/) {
		printf("Unable to rename %s to %s\n", from, to);

		return (-1);
	}

	return (0);
}
