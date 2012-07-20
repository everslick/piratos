#include "cli.h"

void
cmd_echo_help(CLI *cli) {
	cli_print(cli, "'echo' prints all arguments.\n");
}

int
cmd_echo_exec(CLI *cli, char *argv[]) {
	int i, argc = cli_num_args(argv);

	if (argc < 2) {
		cli_print(cli, "\n");

		return (0);
	}

	cli_print(cli, "%d arguments passed to 'echo'", argc - 1);

	cli_print(cli, ":\n");

	for (i = 1; i < argc; i++) cli_print(cli, "[%d] -> %s\n", i, argv[i]);

	return (0);
}
