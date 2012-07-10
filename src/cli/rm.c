#include <string.h>

#include "fat_filelib.h"
#include "getopt.h"

#include "shell.h"

enum {
	RM_UNKNOWN,
	RM_FILE,
	RM_DIR
};

static struct option const long_options[] = {
	{ "recursive", no_argument, 0, 'r' },
	{ "verbose",   no_argument, 0, 'v' },
	{  0,          0,           0,  0  }
};

int
rm_recursive(const char *path, int verbose) {
	FL_DIR dirstat, *dirp = fl_opendir(path, &dirstat);
	char buf[PATH_MAX];
	int r = -1;

	if (dirp) {
		struct fs_dir_ent dirent;

		r = 0;

		while (!r && (fl_readdir(&dirstat, &dirent) == 0)) {
			int r2 = -1;

			/* Skip the names "." and ".." as we don't want to recurse on them. */
			if (!strcmp(dirent.filename, ".") || !strcmp(dirent.filename, "..")) {
				continue;
			}

			sprintf(buf, "%s/%s", path, dirent.filename);

			if (fl_is_dir(buf)) {
				r2 = rm_recursive(buf, verbose);
			} else {
				r2 = fl_remove(buf);

				if (verbose) {
					shell_print("rm: removing '%s'\n", buf);
				}
			}

			r = r2;
		}

		fl_closedir(dirp);
	}

	if (!r) {
		r = fl_remove(path);

		if (verbose) {
			shell_print("rm: removing '%s'\n", path);
		}
	}

	return (r);
}

static int
rm_single(const char *path, int verbose) {
	if (fl_remove(path)) {
		shell_print( "rm: could not remove file '%s'\n", path);

		return (1);
	}

	if (verbose) {
		shell_print("rm: removing '%s'\n", path);
	}

	return (0);
}

static int
rm_scope(const char *path) {
	if (fl_is_dir(path)) {
		return (RM_DIR);
	}

	return (RM_FILE);
}

void
cmd_rm_help(void) {
	shell_print("'rm' removes files and directories.                           \n");
	shell_print("                                                              \n");
	shell_print("Usage:  rm [options] <path>                                   \n");
	shell_print("Options:                                                      \n");
	shell_print("  -r, --recursive  Recursively remove sub directories         \n");
	shell_print("  -v, --verbose    Be extremely noisy about what is happening \n");
}

int
cmd_rm_exec(char *argv[]) {
	int argc = shell_num_args(argv);
	int recursive = 0, verbose = 0;
	char path[PATH_MAX];
	int scope, ret = 0;
	int i, c, opt_ind;

	for (c = 0, optind = 0, opt_ind = 0; c != -1;) {
		c = getopt_long(argc, argv, "rv", long_options, &opt_ind);
		switch (c) {
			case 'r': recursive = 1; break;
			case 'v': verbose   = 1; break;
		}
	}

	if (argc - optind < 2) {
		shell_print("rm: insufficient number of arguments. Try 'man rm'\n");

		return (-1);
	}

	for (i=optind+1; argv[i]; i++) {
		shell_clean_path(argv[i], path);

		scope = rm_scope(path);

		switch (scope) {
			case RM_FILE:
				ret += rm_single(path, verbose);
			break;

			case RM_DIR:
				if (!recursive) {
					shell_print("%s is a directory, use -r to remove it.\n", path);
					ret++;
				} else {
					ret += rm_recursive(path, verbose);
				}
			break;
		}
	}

	if (ret) return (-1);

	return (0);
}
