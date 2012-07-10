#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "shell.h"

#define MAX_COMMAND_NUM  1<<5
#define MAX_COMMAND_LINE 1<<16
#define MAX_ARGUMENT_NUM 1<<8

#define SHELL_REGISTER_COMMAND(_CMD_)    \
	register_command(#_CMD_, cmd_ ## _CMD_ ## _exec, cmd_ ## _CMD_ ## _help);

#define SHELL_COMMAND_PROTOTYPE(_CMD_)   \
	int  cmd_ ## _CMD_ ## _exec(char **); \
	void cmd_ ## _CMD_ ## _help(void);

char shell_prompt[PATH_MAX];
char shell_cwd[PATH_MAX];
char shell_owd[PATH_MAX];
int  shell_error = 0;
int  shell_quit  = 0;

struct {
	const char *name;
	int  (*exec)(char **);
	void (*help)(void);
} shell_commands[MAX_COMMAND_NUM];

SHELL_COMMAND_PROTOTYPE(cat)
SHELL_COMMAND_PROTOTYPE(cd)
SHELL_COMMAND_PROTOTYPE(clean)
SHELL_COMMAND_PROTOTYPE(cp)
SHELL_COMMAND_PROTOTYPE(dd)
SHELL_COMMAND_PROTOTYPE(echo)
SHELL_COMMAND_PROTOTYPE(halt)
SHELL_COMMAND_PROTOTYPE(hd)
SHELL_COMMAND_PROTOTYPE(help)
SHELL_COMMAND_PROTOTYPE(info)
SHELL_COMMAND_PROTOTYPE(ls)
SHELL_COMMAND_PROTOTYPE(mkdir)
SHELL_COMMAND_PROTOTYPE(mv)
SHELL_COMMAND_PROTOTYPE(pwd)
SHELL_COMMAND_PROTOTYPE(quit)
SHELL_COMMAND_PROTOTYPE(rm)
SHELL_COMMAND_PROTOTYPE(touch)

static void
register_command(const char *name, int (*exec)(char **), void (*help)(void)) {
	for (int i=0; i<MAX_COMMAND_NUM; i++) {
		if (!shell_commands[i].name) {
			shell_commands[i].name = name;
			shell_commands[i].exec = exec;
			shell_commands[i].help = help;

			return;
		}
	}
}

static void
register_commands(void) {

	memset(shell_commands, 0, sizeof (shell_commands));

	//SHELL_REGISTER_COMMAND(cat)
	SHELL_REGISTER_COMMAND(cd)
	SHELL_REGISTER_COMMAND(clean)
	//SHELL_REGISTER_COMMAND(cp)
	SHELL_REGISTER_COMMAND(dd)
	SHELL_REGISTER_COMMAND(echo)
	SHELL_REGISTER_COMMAND(halt)
	SHELL_REGISTER_COMMAND(hd)
	SHELL_REGISTER_COMMAND(help)
	SHELL_REGISTER_COMMAND(info)
	SHELL_REGISTER_COMMAND(ls)
	SHELL_REGISTER_COMMAND(mkdir)
	SHELL_REGISTER_COMMAND(mv)
	SHELL_REGISTER_COMMAND(pwd)
	SHELL_REGISTER_COMMAND(quit)
	SHELL_REGISTER_COMMAND(rm)
	SHELL_REGISTER_COMMAND(touch)
}

static int
cmd_exec(char **args) {
	for (int i=0; i<MAX_COMMAND_NUM; i++) {
		if (shell_commands[i].name) {
			if (!strcmp(args[0], shell_commands[i].name)) {
				shell_error = shell_commands[i].exec(args);

				return (0);
			}
		}
	}

	return (-1);
}

static int
cmd_help(char **args) {
	for (int i=0; i<MAX_COMMAND_NUM; i++) {
		if (shell_commands[i].name) {
			if (!strcmp(args[1], shell_commands[i].name)) {
				shell_commands[i].help();

				return (0);
			}
		}
	}

	return (-1);
}

static void
read_line(char *buffer, int size) {
	fgets(buffer, size, stdin);
}

static void
parse_args(char *buffer, char **args, int *nargs) {
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
shell_print(const char *fmt, ...) {
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = vprintf(fmt, ap);
	va_end(ap);

	return (n);
}

int
shell_num_args(char **args) {
	int i = 0;

	while (args[i]) i++;

	return (i);
}

void
shell_get_cwd(char *cwd) {
	snprintf(cwd, PATH_MAX, "%s", shell_cwd);
}

void
shell_get_owd(char *owd) {
	snprintf(owd, PATH_MAX, "%s", shell_owd);
}

void
shell_set_cwd(char *cwd) {
	snprintf(shell_owd, PATH_MAX, "%s", shell_cwd);
	snprintf(shell_cwd, PATH_MAX, "%s", cwd);
}

void
shell_set_prompt(char *prompt) {
	snprintf(shell_prompt, PATH_MAX, "root:%s# ", prompt);
}

void
shell_clean_path(char *path, char *cleaned) {
	char *ptr, *next, *end;
	int res_len, src_len;
	char src[PATH_MAX];

	if (!path) {
		// no path at all
		sprintf(src, "%s", shell_cwd);
	} else if (path[0] == '/') {
		// absolute path
		sprintf(src, "%s", path);
	} else {
		// relative path
		sprintf(src, "%s/%s", shell_cwd, path);
	}

	res_len = 0;
	src_len = strlen(src);
	end = &src[src_len];

	for (ptr=src; ptr<end; ptr=next+1) {
		int len;

		next = memchr(ptr, '/', end - ptr);
		if (!next) next = end;
		len = next - ptr;

		switch (len) {
			case 2:
				if (ptr[0] == '.' && ptr[1] == '.') {
					char *slash = memrchr(cleaned, '/', res_len);

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

int
shell_init(void){
	char buffer[MAX_COMMAND_LINE];
	char *args[MAX_ARGUMENT_NUM];
	int nargs;

	register_commands();

	shell_set_cwd("/");
	shell_set_prompt("/");

	while (!shell_quit) {
		shell_print("%s", shell_prompt);

		read_line(buffer, MAX_COMMAND_LINE);
		parse_args(buffer, args, &nargs);

		if (nargs == 0) continue;

		// special case for man command
		if (!strcmp(args[0], "man")) {
			if (nargs == 2) {
				if (cmd_help(args) != 0) {
					shell_print("man: no manual for command '%s' available\n", args[1]);
				}
			} else {
				shell_print("USAGE: man [command]\n");
			}

			continue;
		}

		if (cmd_exec(args) == 0) continue;

		shell_print("%s: command not found\n", args[0]);
/*
		pid_t pid = fork();

		if (pid) {
			//int *ret_status;

			printf("Waiting for child (%d)\n", pid);
			pid = wait(ret_status);
			printf("Child (%d) finished\n", pid);
		} else {
			if (execvp(args[0], args)) {
				puts(strerror(errno));
				exit(127);
			}
		}
*/
	}

	return (0);
}
