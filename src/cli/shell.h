#ifndef _SHELL_H_
#define _SHELL_H_

#define PATH_MAX (1<<8)

// TODO make a struct
extern char shell_prompt[];
extern char shell_cwd[];
extern char shell_owd[];
extern int  shell_error;
extern int  shell_quit;

int  shell_num_args(char **args);
void shell_clean_path(char *path, char *cleaned);
void shell_set_cwd(char *cwd);
void shell_get_cwd(char *cwd);
void shell_get_owd(char *owd);
void shell_set_prompt(char *prompt);

int shell_init(void);

#endif /* _SHELL_H_ */
