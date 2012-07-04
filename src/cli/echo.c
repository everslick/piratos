#include <stdio.h>

#include "shell.h"

void
cmd_echo_help(void) {
	printf("'echo' prints all arguments.\n");
}

int
cmd_echo_exec(char *argv[]) {
	int i, argc = shell_num_args(argv);

	if (argc < 2) {
		printf("\n");

		return (0);
	}

	printf("%d arguments passed to 'echo'", argc - 1);

	printf(":\n");

	for (i = 1; i < argc; i++) printf("[%d] -> %s\n", i, argv[i]);

	return (0);
}
