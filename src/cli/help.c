#include "shell.h"

void
cmd_help_help(void) {
	shell_print("'help' lists all available shell commands.\n");
}

int
cmd_help_exec(char *argv[]) {
	shell_print("available commands are:\n");
	shell_print("-----------------------------------------------------------\n");
	shell_print("     cat: print file content                               \n");
	shell_print("      cd: change current directory                         \n");
	shell_print("   clean: clean up the given path name                     \n");
	shell_print("      cp: copy file                                        \n");
	shell_print("      dd: copy a file, formatted according to the operands \n");
	shell_print("    echo: print all arguments                              \n");
	shell_print("      hd: hexdump a file to stdout                         \n");
	shell_print("      ls: list (current working) directory                 \n");
	shell_print("   mkdir: create directory                                 \n");
	shell_print("      mv: move and rename files and directories            \n");
	shell_print("     pwd: print (current) working directory                \n");
	shell_print("      rm: delete (remove) file                             \n");
	shell_print("     man: provide detailed help for command                \n");
	shell_print("    help: list available commands                          \n");
	shell_print("    info: OS name and version                              \n");
	shell_print("    quit: terminate shell                                  \n");
	shell_print("    halt: exit OS                                          \n");

	return (0);
}
