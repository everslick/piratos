#include "fat_filelib.h"

#include "shell.h"

void
cmd_touch_help(void) {
	shell_print("'touch' creates emty files, if they don't exist. \n");
	shell_print("                                                 \n");
	shell_print("USAGE:  touch <file>                             \n");
}

int
cmd_touch_exec(char *argv[]) {
	int argc = shell_num_args(argv);
	char path[PATH_MAX];
	int i, err = 0;
	FL_FILE *file;

	if (argc == 1) {
		shell_print("touch - incorrect number of arguments. Try 'man touch'\n");

		return (-1);
	}

	for (i=1; i<argc; i++) {
		shell_clean_path(argv[1], path);

		if (fl_is_dir(path)) {
			shell_print("touch: '%s' is a directory\n", path);
			err++; continue;
		}

		file = fl_fopen(path, "r+");

		if (file) {
			fl_fclose(file);
		} else {
			file = fl_fopen(path, "w");
		}

		if (file) {
			fl_fclose(file);
		} else {
			shell_print("touch: Could not create '%s'\n", path);
			err++; continue;
		}
	}

	return (err ? -1 : 0);
}
