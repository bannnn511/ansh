#include "parse.h"
#include "utils.h"
#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_CMD_LEN 200

#define errExit(msg)                                                           \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

/* No fork mode will run the command in the same process */
int _NOFORK = 0;

/* Execute mode will run hardcode command */
int _EXEC = 0;

/* Shell will be executed in interactive mode */
int _INTERACTIVE_MODE = 1;

/* Array of paths where the shell should look for commands */
char *SHELL_PATH[BUFSIZ] = {"/bin", "/usr/bin", "/opt/homebrew/bin", NULL};

void print_prompt(void);
void print_simple_prompt(void);
void print_debug(char *msg);
void update_path(char **paths);
int execute_command(char *tokens[]);

int main(int const argc, char *argv[]) {
  printf("Welcome to ansh shell, the interactive friendly shell by An\n");

  if (argc > 1) {
    if (strcmp(argv[1], "-d") == 0) {
      _DEBUG = 1;
    } else {

      _INTERACTIVE_MODE = 0;
    }
  }

  if (_INTERACTIVE_MODE == 1) {
    for (;;) {
      char cmd[MAX_CMD_LEN];
      print_prompt();
      fflush(stdout);

      if (fgets(cmd, MAX_CMD_LEN, stdin) == NULL) {
        break;
      }
      print_debug(cmd);
      // remove new line
      if (cmd[strlen(cmd) - 1] == '\n') {
        cmd[strlen(cmd) - 1] = '\0';
      }

      char **tokens = calloc(strlen(cmd), sizeof(char **));
      if (tokens == NULL) {
        errExit("calloc");
      }
      for (unsigned long i = 0; i < strlen(cmd); i++) {
        tokens[i] = calloc(strlen(cmd), sizeof(char));
        if (tokens[i] == NULL) {
          errExit("calloc");
        }
      }
      parse_input(tokens, cmd);
      execute_command(tokens);

      for (unsigned long j = 0; j < strlen(cmd); j++) {
        free(tokens[j]);
      }
      free(tokens);
    }
  } else {
    /* Batch mode */
    // TODO: batch mode
    printf("file name is %s\n", argv[1]);
  }

  return 0;
}

void print_prompt(void) {
  char **dirs = calloc(BUFSIZ, sizeof(char *));
  if (dirs == NULL) {
    perror("calloc");
    return;
  }
  char current_dir[PATH_MAX];

  if (getwd(current_dir) == NULL) {
    perror("getcwd");
    free(dirs);
    return;
  }

  const int i = split_line(dirs, current_dir, "/");
  printf("\n");
  if (i > 1) {
    printf(BLU "%s/%s\n" RESET, dirs[i - 1], dirs[i]);
    printf(GRN "ansh-> " RESET);
  } else if (i > 0) {
    printf(BLU "%s\n" RESET, dirs[i]);
    printf(GRN "ansh-> " RESET);
  } else {
    printf(BLU "/🔒\n" RESET);
    printf(GRN "ansh->%s" RESET, "");
  }

  free(dirs);
}

/* Overwrite the SHELL_PATH lookup with paths */
void update_path(char **paths) {
  *SHELL_PATH = NULL;
  for (char **path = paths; *path; path++) {
    *SHELL_PATH = *path;
    (*SHELL_PATH)++;
  }
}

int search_path(char path[], const char *cmd) {
  int i = 0;
  while (SHELL_PATH[i] != NULL) {
    snprintf(path, BUFSIZ, "%s/%s", SHELL_PATH[i], cmd);
    if (access(path, X_OK) == 0) {
      return 0;
    }
    i++;
  }

  return i;
}

int execute_command(char *tokens[]) {

  // if (i == -1) {
  //   errExit("parse input");
  // }

  char path[BUFSIZ];
  if (search_path(path, tokens[0]) == -1) {
    return -1;
  }

  if (strncmp(tokens[0], "exit", 4) == 0) {
    exit(EXIT_SUCCESS);
  }

  if (strncmp(tokens[0], "cd", 2) == 0) {
    char *dir = tokens[1];
    if (*dir == '\0') {
      fprintf(stderr, "cd: missing operand\n");
      return -1;
    }

    if (chdir(trim(dir)) == -1) {
      perror("chdir");
    }
  }

  if (strncmp(tokens[0], "path", 4) == 0) {
    char **paths = calloc(BUFSIZ, sizeof(char *));
    char *args = tokens[0] + 5;
    split_line(paths, args, " ");
    update_path(paths);
  }

  print_debug("exec cmd\n");
  int result = 0;
  pid_t child;
  int child_pid = 0;
  switch (child = fork()) {
  case -1:
    errExit("fork");
  case 0:
    child_pid = getpid();
    result = execv(path, tokens);
  default:
    waitpid(child, NULL, 0);
    print_debug("child done\n");
    if (result == -1) {
      fprintf(stderr, "ansh: Unknown command: %s\n", tokens[0]);
      kill(child_pid, SIGTERM);
    }
  }

  return -1;
}
