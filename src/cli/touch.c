#include "fat_filelib.h"

#include "cli.h"

void
cmd_touch_help(CLI *cli) {
	printf("'touch' creates emty files, if they don't exist. \n");
	printf("                                                 \n");
	printf("USAGE:  touch <file>                             \n");
}

int
cmd_touch_exec(CLI *cli, char *argv[]) {
	int argc = cli_num_args(argv);
	char path[PATH_MAX];
	int i, err = 0;
	FL_FILE *file;

	if (argc == 1) {
		printf("touch - incorrect number of arguments. Try 'man touch'\n");

		return (-1);
	}

	for (i=1; i<argc; i++) {
		cli_clean_path(cli, argv[1], path);

		if (fl_is_dir(path)) {
			printf("touch: '%s' is a directory\n", path);
			err++; continue;
		}

		file = (FL_FILE *)fl_fopen(path, "r+");

		if (file) {
			fl_fclose(file);
		} else {
			file = (FL_FILE *)fl_fopen(path, "w");
		}

		if (file) {
			fl_fclose(file);
		} else {
			printf("touch: Could not create '%s'\n", path);
			err++; continue;
		}
	}

	return (err ? -1 : 0);
}
