#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "cli.h"

#define CLI_REGISTER_COMMAND(_CMD_)    \
	register_command(#_CMD_, cmd_ ## _CMD_ ## _exec, cmd_ ## _CMD_ ## _help);

#define CLI_COMMAND_PROTOTYPE(_CMD_)   \
	int  cmd_ ## _CMD_ ## _exec(CLI *, char **); \
	void cmd_ ## _CMD_ ## _help(CLI *);

struct CliCommands {
	const char *name;
	int  (*exec)(CLI *, char **);
	void (*help)(CLI *);
} cli_commands[MAX_COMMAND_NUM];

static void register_command(const char *, int (*)(CLI *, char **), void (*)(CLI *));

CLI_COMMAND_PROTOTYPE(cat)
CLI_COMMAND_PROTOTYPE(cd)
CLI_COMMAND_PROTOTYPE(clean)
CLI_COMMAND_PROTOTYPE(cp)
CLI_COMMAND_PROTOTYPE(dd)
CLI_COMMAND_PROTOTYPE(echo)
CLI_COMMAND_PROTOTYPE(halt)
CLI_COMMAND_PROTOTYPE(hd)
CLI_COMMAND_PROTOTYPE(help)
CLI_COMMAND_PROTOTYPE(info)
CLI_COMMAND_PROTOTYPE(ls)
CLI_COMMAND_PROTOTYPE(mkdir)
CLI_COMMAND_PROTOTYPE(mv)
CLI_COMMAND_PROTOTYPE(pwd)
CLI_COMMAND_PROTOTYPE(quit)
CLI_COMMAND_PROTOTYPE(rm)
CLI_COMMAND_PROTOTYPE(touch)

void
cli_register_commands(void) {

	memset(cli_commands, 0, sizeof (cli_commands));

	//CLI_REGISTER_COMMAND(cat)
	CLI_REGISTER_COMMAND(cd)
	CLI_REGISTER_COMMAND(clean)
	//CLI_REGISTER_COMMAND(cp)
	CLI_REGISTER_COMMAND(dd)
	CLI_REGISTER_COMMAND(echo)
	CLI_REGISTER_COMMAND(halt)
	CLI_REGISTER_COMMAND(hd)
	CLI_REGISTER_COMMAND(help)
	CLI_REGISTER_COMMAND(info)
	CLI_REGISTER_COMMAND(ls)
	CLI_REGISTER_COMMAND(mkdir)
	CLI_REGISTER_COMMAND(mv)
	CLI_REGISTER_COMMAND(pwd)
	CLI_REGISTER_COMMAND(quit)
	CLI_REGISTER_COMMAND(rm)
	CLI_REGISTER_COMMAND(touch)
}

static void
register_command(const char *name, int (*exec)(CLI *, char **), void (*help)(CLI *)) {
	for (int i=0; i<MAX_COMMAND_NUM; i++) {
		if (!cli_commands[i].name) {
			cli_commands[i].name = name;
			cli_commands[i].exec = exec;
			cli_commands[i].help = help;

			return;
		}
	}
}

int
cli_cmd_exec(CLI *cli, char **args) {
	for (int i=0; i<MAX_COMMAND_NUM; i++) {
		if (cli_commands[i].name) {
			if (!strcmp(args[0], cli_commands[i].name)) {
				cli->cli_error = cli_commands[i].exec(cli, args);

				return (0);
			}
		}
	}

	return (-1);
}

int
cli_cmd_help(CLI *cli, char **args) {
	for (int i=0; i<MAX_COMMAND_NUM; i++) {
		if (cli_commands[i].name) {
			if (!strcmp(args[1], cli_commands[i].name)) {
				cli_commands[i].help(cli);

				return (0);
			}
		}
	}

	return (-1);
}

void
cli_parse_args(char *buffer, char **args, int *nargs) {
	char *buf_args[MAX_ARGUMENT_NUM];
	char **cp, *wbuf;
	int i, j;

	wbuf = buffer;
	buf_args[0] = buffer; 
	args[0] = buffer;

	for (cp=buf_args; (*cp=strsep(&wbuf, " \n\t"));) {
		if ((*cp != '\0') && (++cp >= &buf_args[MAX_ARGUMENT_NUM])) break;
	}

	for (j=i=0; buf_args[i]; i++) {
		if (strlen(buf_args[i]) > 0) args[j++] = buf_args[i];
	}

	*nargs = j;
	args[j] = NULL;
}

int
cli_print(CLI *cli, const char *fmt, ...) {
	char buffer[4096];
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = vsnprintf(buffer, sizeof (buffer), fmt, ap);
	va_end(ap);

	cli->vt->Write(buffer, n);

	return (n);
}

int
cli_num_args(char **args) {
	int i = 0;

	while (args[i]) i++;

	return (i);
}

void
cli_get_cwd(CLI *cli, char *cwd) {
	snprintf(cwd, PATH_MAX, "%s", cli->cli_cwd);
}

void
cli_get_owd(CLI *cli, char *owd) {
	snprintf(owd, PATH_MAX, "%s", cli->cli_owd);
}

void
cli_set_cwd(CLI *cli, char *cwd) {
	snprintf(cli->cli_owd, PATH_MAX, "%s", cli->cli_cwd);
	snprintf(cli->cli_cwd, PATH_MAX, "%s", cwd);
}

void
cli_set_prompt(CLI *cli, char *prompt) {
	snprintf(cli->cli_prompt, PATH_MAX, "root:%s# ", prompt);
}

void
cli_clean_path(CLI *cli, char *path, char *cleaned) {
	char *ptr, *next, *end;
	int res_len, src_len;
	char src[PATH_MAX];

	if (!path) {
		// no path at all
		sprintf(src, "%s", cli->cli_cwd);
	} else if (path[0] == '/') {
		// absolute path
		sprintf(src, "%s", path);
	} else {
		// relative path
		sprintf(src, "%s/%s", cli->cli_cwd, path);
	}

	res_len = 0;
	src_len = strlen(src);
	end = &src[src_len];

	for (ptr=src; ptr<end; ptr=next+1) {
		int len;

		next = (char *)memchr(ptr, '/', end - ptr);
		if (!next) next = end;
		len = next - ptr;

		switch (len) {
			case 2:
				if (ptr[0] == '.' && ptr[1] == '.') {
					char *slash = (char *)memrchr(cleaned, '/', res_len);

					if (slash) res_len = slash - cleaned;

					continue;
				}
			break;

			case 1:
				if (ptr[0] == '.') continue;
			break;

			case 0: continue;
		}

		cleaned[res_len++] = '/';
		memcpy(&cleaned[res_len], ptr, len);
		res_len += len;
	}

	if (!res_len) cleaned[res_len++] = '/';

	cleaned[res_len] = '\0';
}

CLI *
cli_new(VT102 *vt102) {
	CLI *cli = (CLI *)malloc(sizeof (CLI));

	cli_set_cwd(cli, (char *)"/");
	cli_set_prompt(cli, (char *)"/");

	cli->vt = vt102;

	return (cli);
}

void
cli_free(CLI *cli) {
	free(cli);
}
