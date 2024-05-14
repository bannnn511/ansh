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

int exec_child(char *cmd, char *argv[]);

int main(int argc, char *argv[]) {
  printf("Welcome to An shell, the interactive friendly shell by An\n");
  char cmd[MAX_CMD_LEN];

  for (;;) {
    printf("-> ");
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
      if (exec_child(cmd, argv) == -1) {
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

int exec_child(char *cmd, char *argv[]) {
  if (execl("/bin/ls", "ls", ".", NULL) == -1) {
    return -1;
  }
  return 0;
}
