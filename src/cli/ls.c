#include "fat_filelib.h"

#include "shell.h"

void
cmd_ls_help(void) {
	shell_print("'ls' lists files and directories.                          \n");
	shell_print("                                                           \n");
	shell_print("If no path is given the current working directory is used. \n");
}

void
list_directory(const char *path) {
    FL_DIR dirstat;

    if (fl_opendir(path, &dirstat)) {
        struct fs_dir_ent dirent;

        while (fl_readdir(&dirstat, &dirent) == 0) {
            if (dirent.is_dir) {
                shell_print("%s \t<DIR>\n", dirent.filename);
            } else {
                shell_print("%s \t[%d bytes]\n", dirent.filename, (int)dirent.size);
            }
        }

        fl_closedir(&dirstat);
    }
}

int
cmd_ls_exec(char *argv[]) {
	int argc = shell_num_args(argv);
	char path[PATH_MAX];

	if (argc > 2) {
		shell_print("Too many arguments to 'ls'\n");

		return (-1);
	}

	if (argc < 2) {
		shell_clean_path(NULL, path); // cwd
	} else {
		shell_clean_path(argv[1], path);
	}

	if (!fl_is_dir(path)) {
		shell_print("Invalid directory '%s'\n", argv[1]);

		return (-1);
	}

	list_directory(path);

	return (0);
}
