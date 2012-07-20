#ifndef _CLI_H_
#define _CLI_H_

#include <term/vt102.h>

#define PATH_MAX (1<<8) // 256 bytes

#define MAX_COMMAND_NUM  1<<5
#define MAX_COMMAND_LINE 1<<16
#define MAX_ARGUMENT_NUM 1<<8

typedef struct {
	char cli_prompt[PATH_MAX];
	char cli_cwd[PATH_MAX];
	char cli_owd[PATH_MAX];
	int  cli_error;
	int  cli_quit;

	VT102 *vt;
} CLI;

#ifdef __cplusplus
//extern "C" {
#endif

void cli_register_commands(void);
int  cli_num_args(char **args);

int  cli_print(CLI *cli, const char *fmt, ...);
void cli_clean_path(CLI *cli, char *path, char *cleaned);
void cli_parse_args(char *buffer, char **args, int *nargs);

int  cli_cmd_exec(CLI *cli, char **args);
int  cli_cmd_help(CLI *cli, char **args);

void cli_set_cwd(CLI *cli, char *cwd);
void cli_get_cwd(CLI *cli, char *cwd);
void cli_get_owd(CLI *cli, char *owd);
void cli_set_prompt(CLI *cli, char *prompt);

CLI *cli_new(VT102 *vt102);
void cli_destroy(CLI *cli);

#ifdef __cplusplus
//}
#endif

#endif /* _CLI_H_ */
