#include "parse.h"
#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"

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

/* Array of paths where the shell should look for commands */
char *SHELL_PATH[BUFSIZ] = {"/bin/", "/usr/bin/", NULL};

void print_prompt(void);
void print_simple_prompt(void);
void print_debug(char *msg);
void update_path(char **paths);
int exec_cmd(const char *cmd);

int main(int const argc, char *argv[]) {
  printf("Welcome to ansh shell, the interactive friendly shell by An\n");
  char cmd[MAX_CMD_LEN];

  if (argc > 1) {
    if (strcmp(argv[1], "-d") == 0) {
      _DEBUG = 1;
    }
  }

  if (argc > 1) {
    if (strcmp(argv[1], "-e") == 0) {
      execl("/usr/bin/git", "git", "add", ".", NULL);
    }
  }

  if (argc > 2) {
    if (strcmp(argv[2], "-nf") == 0) {
      _NOFORK = 1;
    }
  }

  // DEBUG MODE
  if (_NOFORK == 1) {
    print_prompt();
    if (fgets(cmd, MAX_CMD_LEN, stdin) == NULL) {
      return -1;
    }
    exec_cmd(cmd);
    return 0;
  }

  for (;;) {
    print_prompt();
    fflush(stdout);

    if (fgets(cmd, MAX_CMD_LEN, stdin) == NULL) {
      break;
    }
    print_debug(cmd);

    if (strncmp(cmd, "exit", 4) == 0) {
      return 0;
    }

    if (strncmp(cmd, "cd", 2) == 0) {
      char *dir = cmd + 3;
      if (*dir == '\0') {
        fprintf(stderr, "cd: missing operand\n");
        continue;
      }

      if (chdir(trim(dir)) == -1) {
        perror("chdir");
      }
      continue;
    }

    if (strncmp(cmd, "path", 4) == 0) {
      char **paths = calloc(BUFSIZ, sizeof(char *));
      char *path = cmd + 5;
      split_line(paths, path, " ");
      update_path(paths);
      continue;
    }

    int result = 0;
    pid_t child;
    int child_pid = 0;
    switch (child = fork()) {
    case -1:
      errExit("fork");
    case 0:
      child_pid = getpid();
      result = exec_cmd(cmd);
    default:
      waitpid(child, NULL, 0);
      print_debug("child done\n");
      if (result == -1) {
        fprintf(stderr, "ansh: Unknown command: %s\n", cmd);
        kill(child_pid, SIGTERM);
      }
    }
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

int exec_cmd(const char *cmd) {
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
  const int i = parse_input(tokens, cmd);
  if (i == -1) {
    errExit("parse input");
  }

  const char *base_cmd = tokens[0];
  for (char **path = SHELL_PATH; *path; path++) {
    char *cmd_path = calloc(
        strlen(*path) + strlen(base_cmd) + 1,
        sizeof(char)
        );
    if (cmd_path == NULL) {
      errExit("calloc");
    }
    strcpy(cmd_path, *path);
    strcat(cmd_path, base_cmd);

    // check path accessibility
    if (access(cmd_path, X_OK) == -1) {
      char buffer[20];
      sprintf(buffer, "%s\n", cmd_path);
      print_debug(buffer);
      free(cmd_path);
      continue;
    }

    if (_DEBUG) {
      char buffer[30];
      sprintf(buffer, "tokens arguments: %d\n", i);
      print_debug(buffer);
    }

    if (i == 0) {
      tokens[0] = "";
      tokens[1] = NULL;
    }

    print_debug("exec cmd\n");
    execv(cmd_path, tokens);
    free(cmd_path);
    for (unsigned long j = 0; j < strlen(cmd); j++) {
      free(tokens[j]);
    }
    perror("exec_child");
  }

  free(tokens);

  return -1;
}
