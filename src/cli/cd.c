#include <string.h>

#include "fat_filelib.h"

#include "cli.h"

void
cmd_cd_help(CLI *cli) {
	cli_print(cli, "'cd' changes the current working directory.\n");
}

int
cmd_cd_exec(CLI *cli, char *argv[]) {
	int argc = cli_num_args(argv);
	char path[PATH_MAX];

	if (argc > 2) {
		cli_print(cli, "Too many arguments to 'cd'\n");

		return (-1);
	}

	if (argc < 2) {
		cli_print(cli, "No directory specified. Try 'man cd'\n");

		return (-1);
	}

	if (!strcmp(argv[1], "-")) {
		cli_get_owd(cli, path);
	} else {
		cli_clean_path(cli, argv[1], path);
	}

	if (!fl_is_dir(path)) {
		cli_print(cli, "Invalid directory '%s'\n", path);

		return (-1);
	}

	cli_set_cwd(cli, path);
	cli_set_prompt(cli, path);

	return (0);
}
