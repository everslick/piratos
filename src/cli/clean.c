#include "cli.h"

void
cmd_clean_help(CLI *cli) {
	cli_print(cli, "'clean' returns the canonicalised absolute pathname.\n");
	cli_print(cli, "\n");
	cli_print(cli, "'clean' converts the given argument to an absolute pathname, which  \n");
	cli_print(cli, "has no components that are the special . or .. directory entries.   \n");
	cli_print(cli, "In addition it removes all repeating and trailing / characters.     \n");
	cli_print(cli, "If the given path argument is relative (i.e. does not start with    \n");
	cli_print(cli, "'/'), 'clean' prepends to it the current directory name as obtained \n");
	cli_print(cli, "from the 'pwd' command before further processing.                   \n");
}

int
cmd_clean_exec(CLI *cli, char *argv[]) {
	int argc = cli_num_args(argv);
	char path[PATH_MAX];

	if (argc > 2) {
		cli_print(cli, "Too many arguments to 'clean'\n");

		return (-1);
	}

	if (argc < 2) {
		cli_print(cli, "No path specified. Try 'man clean'\n");

		return (-1);
	}

	cli_clean_path(cli, argv[1], path);

	cli_print(cli, "%s\n", path);

	return (0);
}
