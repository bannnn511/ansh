#ifndef MAIN_H
#define MAIN_H
#include <stdio.h>

#define MAX_CMD_LEN 200

#define errExit(msg)                                                           \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)


/* Shell will be executed in interactive mode */
int _INTERACTIVE_MODE = 1;

/* Array of paths where the shell should look for commands */
char* SHELL_PATH[BUFSIZ] = {"/bin", "/usr/bin", "/opt/homebrew/bin", NULL};

void print_prompt(void);
void print_simple_prompt(void);
void print_debug(char* msg);
void update_path(char** paths);
int execute_command(char* tokens[],int is_redirect, char out[]);
void redirect(FILE* out, FILE* temp);


#endif // MAIN_H
