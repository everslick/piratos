#include "shell.h"

void
cmd_clean_help(void) {
	shell_print("'clean' returns the canonicalised absolute pathname.\n");
	shell_print("\n");
	shell_print("'clean' converts the given argument to an absolute pathname, which  \n");
	shell_print("has no components that are the special . or .. directory entries.   \n");
	shell_print("In addition it removes all repeating and trailing / characters.     \n");
	shell_print("If the given path argument is relative (i.e. does not start with    \n");
	shell_print("'/'), 'clean' prepends to it the current directory name as obtained \n");
	shell_print("from the 'pwd' command before further processing.                   \n");
}

int
cmd_clean_exec(char *argv[]) {
	int argc = shell_num_args(argv);
	char path[PATH_MAX];

	if (argc > 2) {
		shell_print("Too many arguments to 'clean'\n");

		return (-1);
	}

	if (argc < 2) {
		shell_print("No path specified. Try 'man clean'\n");

		return (-1);
	}

	shell_clean_path(argv[1], path);

	shell_print("%s\n", path);

	return (0);
}
