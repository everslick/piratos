#include <stdio.h>
#include <string.h>

#include "fat_filelib.h"
#include "getopt.h"

#include "shell.h"

static struct option const long_options[] = {
	{ "parents", no_argument, 0, 'p'},
	{ "verbose", no_argument, 0, 'v'},
	{  0,        0,           0,  0 }
};

void
cmd_mkdir_help(void) {
	printf("'mkdir' creates a new directory.                        \n\n");
	printf(
		"Usage: mkdir [options] <path>                                 \n"
		"Options:                                                      \n"
		"  -p, --parents    Create needed parents for <path>           \n"
		"  -v, --verbose    Be extremely noisy about what is happening \n"
	);
}

static int
create_directory(char *path, int parents, int verbose) {
	char dir[PATH_MAX], buf[PATH_MAX];
	char *ptr, *save, *dirs[255];
	FL_DIR dirstat, *dirp;
	int i = 0;

	shell_clean_path(path, dir);

	if (!parents) {
		dirp = fl_opendir(dir, &dirstat);

		if (dirp) {
			printf("mkdir: cannot create directory '%s': File exists\n", dir);
			fl_closedir(dirp);
			return (-1);
		}

		if ((fl_createdirectory(dir) == -1)) {
			printf("mkdir: could not create '%s', try '-p'\n", dir);
			return (-1);
		}

		if (verbose) {
			printf("mkdir: created directory '%s'\n", dir);
		}
	} else {
		dirs[i] = strtok_r(dir, "/", &save);
		while (dirs[i] && (i < 255)) {
			dirs[++i] = strtok_r(NULL, "/", &save);
		}

		ptr = buf; i = 0;

		while (dirs[i]) {
			ptr += sprintf(ptr, "/%s", dirs[i]);

			if (fl_createdirectory(buf)) {
				printf("mkdir: could not create '%s'\n", buf);
			} else {
				if (verbose) {
					printf("mkdir: created directory '%s'\n", buf);
				}
			}

			i++;
		}
	}

	return (0);
}

int
cmd_mkdir_exec(char *argv[]) {
	int argc = shell_num_args(argv);
	int verbose = 0, parents = 0;
	int i, c, opt_ind;

	for (c = 0, optind = 0, opt_ind = 0; c != -1;) {
		c = getopt_long(argc, argv, "pv", long_options, &opt_ind);

		switch (c) {
			case 'p': parents = 1; break;
			case 'v': verbose = 1; break;
		}
	}

	if (argc - optind < 2) {
		printf("mkdir - incorrect number of arguments. Try 'man mkdir'\n");

		return (-1);
	}

	for (i=optind+1; argv[i]; i++) {
		create_directory(argv[i], parents, verbose);
	}

	return (0);
}
