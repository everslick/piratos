#include "cli.h"

void
cmd_help_help(CLI *cli) {
	cli_print(cli, "'help' lists all available shell commands.\n");
}

int
cmd_help_exec(CLI *cli, char *argv[]) {
	cli_print(cli, "available commands are:\n");
	cli_print(cli, "-----------------------------------------------------------\n");
	cli_print(cli, "     cat: print file content                               \n");
	cli_print(cli, "      cd: change current directory                         \n");
	cli_print(cli, "   clean: clean up the given path name                     \n");
	cli_print(cli, "      cp: copy file                                        \n");
	cli_print(cli, "      dd: copy a file, formatted according to the operands \n");
	cli_print(cli, "    echo: print all arguments                              \n");
	cli_print(cli, "      hd: hexdump a file to stdout                         \n");
	cli_print(cli, "      ls: list (current working) directory                 \n");
	cli_print(cli, "   mkdir: create directory                                 \n");
	cli_print(cli, "      mv: move and rename files and directories            \n");
	cli_print(cli, "     pwd: print (current) working directory                \n");
	cli_print(cli, "      rm: delete (remove) file                             \n");
	cli_print(cli, "     man: provide detailed help for command                \n");
	cli_print(cli, "    help: list available commands                          \n");
	cli_print(cli, "    info: OS name and version                              \n");
	cli_print(cli, "    quit: terminate shell                                  \n");
	cli_print(cli, "    halt: exit OS                                          \n");

	return (0);
}
