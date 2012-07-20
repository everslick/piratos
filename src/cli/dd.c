#include <string.h>

#include "fat_filelib.h"
#include "getopt.h"

#include "cli.h"

static struct option const long_options[] = {
	{ "verbose", no_argument,       0, 'v'},
	{ "infile",  required_argument, 0, 'i'},
	{ "outfile", required_argument, 0, 'o'},
	{ "block",   required_argument, 0, 'b'},
	{ "count",   required_argument, 0, 'c'},
	{ "skip",    required_argument, 0, 's'},
	{ "seek",    required_argument, 0, 'k'},
	{  0,        0,                 0,  0 }
};

void
cmd_dd_help(CLI *cli) {
	cli_print(cli, "'dd' dump and format file contents.                             \n");
	cli_print(cli, "                                                                \n");
	cli_print(cli, "Usage:  dd -i infile -o outfile [-b block] [-c count] [-s skip] \n");
	cli_print(cli, "                                [-k seek] [-v]                  \n");
}

static int
atoi(const char *string) {
	int i = 0;

	while (*string) {
		i = (i<<3) + (i<<1) + (*string - '0');
		string++;

		// don't increment i!

	}

	return (i);
}

int
cmd_dd_exec(CLI *cli, char *argv[]) {
	int verbose = 0, seek = 0, skip = 0, count = -1, block = 32;
	int size, rc, err = 0, argc = cli_num_args(argv);
	char infile[PATH_MAX], outfile[PATH_MAX];
	char *inname = NULL, *outname = NULL;
	FL_FILE *in = NULL, *out = NULL;
	int c, opt_ind;
	//char *buffer = NULL;

	for (c = 0, optind = 0, opt_ind = 0; c != -1;) {
		c = getopt_long(argc, argv, "vi:o:b:c:s:k:", long_options, &opt_ind);

		switch (c) {
			case 'v': verbose =           1;  break;
			case 'i': inname  =      optarg;  break;
			case 'o': outname =      optarg;  break;
			case 'b': block   = atoi(optarg); break;
			case 's': skip    = atoi(optarg); break;
			case 'c': count   = atoi(optarg); break;
			case 'k': seek    = atoi(optarg); break;
		}
	}

	if (verbose) {
		cli_print(cli, "inname  = %s\n", inname);
		cli_print(cli, "outname = %s\n", outname);
		cli_print(cli, "block   = %i\n", block);
		cli_print(cli, "skip    = %i\n", skip);
		cli_print(cli, "seek    = %i\n", seek);
		cli_print(cli, "count   = %i\n", count);
	}

	//buff = (char *)mall c(block);
	char buffer[block];

	if (argc - optind != 1) {
		cli_print(cli, "dd - incorrect number of arguments. Try 'man dd'\n");
		err++; goto cleanup;
	}

	if (buffer == NULL) {
		cli_print(cli, "dd: out of memory.\n");
		err++; goto cleanup;
	}

	if (inname) {
		cli_clean_path(cli, inname, infile);

		if (fl_is_dir(infile)) {
			cli_print(cli, "dd: '%s' is a directory\n", infile);
			err++; goto cleanup;
		}

		in = (FL_FILE *)fl_fopen(infile, "r");

		if (!in) {
			cli_print(cli, "dd: could not open '%s' for reading\n", infile);
			err++; goto cleanup;
		}

		fl_fseek(in, skip * block, SEEK_SET);
	} else {
		cli_print(cli, "dd: no input file specified - dumping zeros\n");
		memset(buffer, 0, block);
	}

	if (outname) {
		cli_clean_path(cli, outname, outfile);

		if (fl_is_dir(outfile)) {
			cli_print(cli, "dd: '%s' is a directory\n", outfile);
			err++; goto cleanup;
		}

		out = (FL_FILE *)fl_fopen(outfile, "r+");

		if (out) {
			rc = fl_fseek(out, seek * block, SEEK_SET);
			cli_print(cli, "WARNING: seeking in outfile beyond EOF is unsupported!\n");
		} else {
			out = (FL_FILE *)fl_fopen(outfile, "w");
		}

		if (!out) {
			cli_print(cli, "dd: could not open '%s' for writeing\n", outfile);
			err++; goto cleanup;
		}
	} else {
		cli_print(cli, "dd: no output file specified - using stdout\n");
	}

	if (count >= 0) {
		size = count;
	} else {
		// TODO size of (infile - skip) / block
	}

	while (size > 0) {
		int blk = count - size;

		if (verbose) cli_print(cli, "dd:  reading block %i\n", blk);

		if (in) {
			rc = fl_fread(buffer, block, 1, in);

			if (rc < 0) {
				cli_print(cli, "dd: Error reading block %i\n", blk);
				err++; goto cleanup;
			} else {
				// short read -> set block size to number of bytes read
				block = rc;
			}
		}

		if (verbose) cli_print(cli, "dd: writeing block %i\n", blk);

		if (out) {
			rc = fl_fwrite(buffer, block, 1, out);

			if (rc < 0) {
				cli_print(cli, "dd: Error writeing block %i\n", blk);
				err++; goto cleanup;
			}
		} else {
			cli_print(cli, "%s\n", buffer);
		}

		size--;
	}

	// TODO print summary

cleanup:
	fl_fclose(in);
	fl_fclose(out);
	//free(buffer);

	return (err ? -1 : 0);
}
