#include "shell.h"

void
cmd_echo_help(void) {
	shell_print("'echo' prints all arguments.\n");
}

int
cmd_echo_exec(char *argv[]) {
	int i, argc = shell_num_args(argv);

	if (argc < 2) {
		shell_print("\n");

		return (0);
	}

	shell_print("%d arguments passed to 'echo'", argc - 1);

	shell_print(":\n");

	for (i = 1; i < argc; i++) shell_print("[%d] -> %s\n", i, argv[i]);

	return (0);
}
