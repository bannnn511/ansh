#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define errExit(msg)                                                           \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#define MAX_CMD_LEN 200

char *SHELL_PATH[BUFSIZ] = {"/bin/", "/usr/bin/", NULL};

int exec_child(char *cmd);

int main(int argc, char *argv[]) {
  printf("Welcome to ansh shell, the interactive friendly shell by An\n");
  char cmd[MAX_CMD_LEN];

  for (;;) {
    printf("ansh-> ");
    fflush(stdout);

    if (fgets(cmd, MAX_CMD_LEN, stdin) == NULL) {
      break;
    }

    if (strncmp(cmd, "exit", 4) == 0) {
      return 0;
    }

    pid_t child;
    switch (child = fork()) {
      case -1:
        errExit("fork");
      case 0:
        if (exec_child(cmd) == -1) {
          errExit("exec_child");
        }
      default:
        if (waitpid(child, NULL, 0) == -1) {
          errExit("waitpid");
        }
    }
  }

  return 0;
}

int exec_child(char *cmd) {
  int i = 0;
  char *tokens[BUFSIZ];
  char **token_ptr = tokens;

  while ((*token_ptr = strsep(&cmd, " ")) != NULL) {
    if (**token_ptr != '\0') {
      token_ptr++;
      i++;
    }
  }
  tokens[i] = NULL;

  char **path;
  char *base_cmd = tokens[0];
  for (path = SHELL_PATH; *path; path++) {
    char *cmd_path = malloc(strlen(*path) + strlen(base_cmd) + 1);
    if (cmd_path == NULL) {
      perror("malloc");
      return -1;
    }
    strcpy(cmd_path, *path);
    strcat(cmd_path, base_cmd);
    if (access(cmd_path, X_OK) == -1) {
      free(cmd_path);
      continue;
    }

    if (execv(cmd_path, tokens) == -1) {
      perror("exec_child");
      free(cmd_path);
      return -1;
    }

    free(cmd_path);
  }

  fprintf(stderr, "%s: command not found\n", base_cmd);
  return -1;
}
