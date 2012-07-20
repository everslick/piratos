#include "fat_filelib.h"

#include "cli.h"

void
cmd_hd_help(CLI *cli) {
	cli_print(cli, "'hd' hexdump the given file to stdout.             \n");
	cli_print(cli, "                                                   \n");
	cli_print(cli, "USAGE:  hd <file>                                  \n");
}

int
cmd_hd_exec(CLI *cli, char *argv[]) {
	int argc = cli_num_args(argv);
	char buffer[16], path[PATH_MAX];
	int eof = 0, rc, line = 0, i;
	FL_FILE *file;

	if (argc != 2) {
		cli_print(cli, "hd - incorrect number of arguments. Try 'man hd'\n");

		return (-1);
	}

	cli_clean_path(cli, argv[1], path);

	if (fl_is_dir(path)) {
		cli_print(cli, "hd: '%s' is a directory\n", path);

		return (-1);
	}

	file = (FL_FILE *)fl_fopen(path, "r");

	if (!file) {
		cli_print(cli, "hd: Could not open '%s'\n", path);

		return (-1);
	}

	while (rc != EOF) {
		rc = fl_fread(buffer, 16, 1, file);

		cli_print(cli, "%04hhx:%04hhx  ", line * 16, line * 16 + rc -1);

		for (i=0; i<rc; i++) {
			cli_print(cli, "%02hhx ", buffer[i]);
			if (i == 7) cli_print(cli, " ");
		}

		for (i=rc; i<16; i++) {
			cli_print(cli, "   ");
			if (i == 7) cli_print(cli, " ");
		}

		cli_print(cli, " |");
		for (i=0; i<rc; i++) {
			if ((buffer[i] < 32) || (buffer[i] > 127)) {
				cli_print(cli, ".");
			} else {
				cli_print(cli, "%c", buffer[i]);
			}
		}
		cli_print(cli, "|\n");

		if (rc < 16) break;

		line++;
	}

	fl_fclose(file);

	return (0);
}
