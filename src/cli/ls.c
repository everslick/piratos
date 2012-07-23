#include "fat_filelib.h"

#include "cli.h"

void
cmd_ls_help(CLI *cli) {
	printf("'ls' lists files and directories.                          \n");
	printf("                                                           \n");
	printf("If no path is given the current working directory is used. \n");
}

void
list_directory(CLI *cli, const char *path) {
    FL_DIR dirstat;

    if (fl_opendir(path, &dirstat)) {
        struct fs_dir_ent dirent;

        while (fl_readdir(&dirstat, &dirent) == 0) {
            if (dirent.is_dir) {
                printf("%s \t<DIR>\n", dirent.filename);
            } else {
                printf("%s \t[%d bytes]\n", dirent.filename, (int)dirent.size);
            }
        }

        fl_closedir(&dirstat);
    }
}

int
cmd_ls_exec(CLI *cli, char *argv[]) {
	int argc = cli_num_args(argv);
	char path[PATH_MAX];

	if (argc > 2) {
		printf("Too many arguments to 'ls'\n");

		return (-1);
	}

	if (argc < 2) {
		cli_clean_path(cli, NULL, path); // cwd
	} else {
		cli_clean_path(cli, argv[1], path);
	}

	if (!fl_is_dir(path)) {
		printf("Invalid directory '%s'\n", argv[1]);

		return (-1);
	}

	list_directory(cli, path);

	return (0);
}
