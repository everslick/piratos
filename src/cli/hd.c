#include "fat_filelib.h"

#include "shell.h"

void
cmd_hd_help(void) {
	shell_print("'hd' hexdump the given file to stdout.             \n");
	shell_print("                                                   \n");
	shell_print("USAGE:  hd <file>                                  \n");
}

int
cmd_hd_exec(char *argv[]) {
	int argc = shell_num_args(argv);
	char buffer[16], path[PATH_MAX];
	int eof = 0, rc, line = 0, i;
	FL_FILE *file;

	if (argc != 2) {
		shell_print("hd - incorrect number of arguments. Try 'man hd'\n");

		return (-1);
	}

	shell_clean_path(argv[1], path);

	if (fl_is_dir(path)) {
		shell_print("hd: '%s' is a directory\n", path);

		return (-1);
	}

	file = fl_fopen(path, "r");

	if (!file) {
		shell_print("hd: Could not open '%s'\n", path);

		return (-1);
	}

	while (rc != EOF) {
		rc = fl_fread(buffer, 16, 1, file);

		shell_print("%04hhx:%04hhx  ", line * 16, line * 16 + rc -1);

		for (i=0; i<rc; i++) {
			shell_print("%02hhx ", buffer[i]);
			if (i == 7) shell_print(" ");
		}

		for (i=rc; i<16; i++) {
			shell_print("   ");
			if (i == 7) shell_print(" ");
		}

		shell_print(" |");
		for (i=0; i<rc; i++) {
			if ((buffer[i] < 32) || (buffer[i] > 127)) {
				shell_print(".");
			} else {
				shell_print("%c", buffer[i]);
			}
		}
		shell_print("|\n");

		if (rc < 16) break;

		line++;
	}

	fl_fclose(file);

	return (0);
}
