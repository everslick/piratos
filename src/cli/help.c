#include <stdio.h>

#include "shell.h"

void
cmd_help_help(void) {
	printf("'help' lists all available shell commands.\n");
}

int
cmd_help_exec(char *argv[]) {
	printf("available commands are:\n");
	printf("-----------------------------------------------------------\n");
	printf("     cat: print file content                               \n");
	printf("      cd: change current directory                         \n");
	printf("   clean: clean up the given path name                     \n");
	printf("      cp: copy file                                        \n");
	printf("      dd: copy a file, formatted according to the operands \n");
	printf("    echo: print all arguments                              \n");
	printf("      ls: list (current working) directory                 \n");
	printf("   mkdir: create directory                                 \n");
	printf("  mkfile: create file with size                            \n");
	printf("      mv: move and rename files and directories            \n");
	printf("     pwd: print (current) working directory                \n");
	printf("      rm: delete (remove) file                             \n");
	printf("     man: provide detailed help for command                \n");
	printf("    help: list available commands                          \n");
	printf("    info: OS name and version                              \n");
	printf("    quit: terminate shell                                  \n");
	printf("    halt: exit OS                                          \n");

	return (0);
}
