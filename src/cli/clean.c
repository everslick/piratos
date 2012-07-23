#include "cli.h"

void
cmd_clean_help(CLI *cli) {
	printf("'clean' returns the canonicalised absolute pathname.\n");
	printf("\n");
	printf("'clean' converts the given argument to an absolute pathname, which  \n");
	printf("has no components that are the special . or .. directory entries.   \n");
	printf("In addition it removes all repeating and trailing / characters.     \n");
	printf("If the given path argument is relative (i.e. does not start with    \n");
	printf("'/'), 'clean' prepends to it the current directory name as obtained \n");
	printf("from the 'pwd' command before further processing.                   \n");
}

int
cmd_clean_exec(CLI *cli, char *argv[]) {
	int argc = cli_num_args(argv);
	char path[PATH_MAX];

	if (argc > 2) {
		printf("Too many arguments to 'clean'\n");

		return (-1);
	}

	if (argc < 2) {
		printf("No path specified. Try 'man clean'\n");

		return (-1);
	}

	cli_clean_path(cli, argv[1], path);

	printf("%s\n", path);

	return (0);
}
