#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define errExit(msg)    \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define MAX_CMD_LEN 200
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

/* Debug mode will print prompt to command line */
int _DEBUG = 0;

/* No fork mode will run the command in the same process */
int _NOFORK = 0;

/* Array of paths where the shell should look for commands */
char *SHELL_PATH[BUFSIZ] = {"/bin/", "/usr/bin/", NULL};

char CURRENT_DIR[PATH_MAX];

/* Length of the SHELL_PATH array */

char *trim(char *s) {
  while (isspace(*s)) {
    s++;
  }

  if (*s == '\0') {
    return s;
  }

  char *end = s + strlen(s) - 1;
  while (end > s && isspace(*end)) {
    end--;
  }

  end[1] = '\0';
  return s;
}

void print_debug(char *msg) {
  if (_DEBUG == 0) {
    return;
  }
  printf("ansh(debug)-> %s\n", msg);
}

int parse_input(char **tokens, char *cmd) {
  char *token;
  int i = 0;
  while ((token = strsep(&cmd, " ")) != NULL) {
    if (*token == '\0') {
      break;
    }
    tokens[i] = trim(token);
    i++;
  }

  return i - 1;
}

int exec_cmd(char *cmd) {
  char **tokens = malloc(sizeof(char *) * BUFSIZ);
  parse_input(tokens, cmd);

  const char *base_cmd = tokens[0];
  for (char **path = SHELL_PATH; *path; path++) {
    char *cmd_path = malloc(strlen(*path) + strlen(base_cmd) + 1);
    if (cmd_path == NULL) {
      errExit("malloc");
    }
    strcpy(cmd_path, *path);
    strcat(cmd_path, base_cmd);

    // check if path
    if (access(cmd_path, X_OK) == -1) {
      print_debug(cmd_path);
      free(cmd_path);
      continue;
    }

    execv(cmd_path, tokens);
    perror("exec_child");
    free(cmd_path);
  }

  return -1;
}

/* Overwrite the SHELL_PATH lookup with paths */
void update_path(char **paths) {
  *SHELL_PATH = NULL;
  for (char **path = paths; *path; path++) {
    *SHELL_PATH = *path;
    (*SHELL_PATH)++;
  }
}

int parse_dir(char **tokens, char *cmd) {
  char *token;
  int i = 0;
  while ((token = strsep(&cmd, "/")) != NULL) {
    if (*token == '\0') {
      continue;
      ;
    }
    tokens[i++] = trim(token);
  }

  return i - 1;
}

void print_prompt() {
  char **dirs = malloc(sizeof(char *) * BUFSIZ);
  getcwd(CURRENT_DIR, PATH_MAX);
  const int i = parse_dir(dirs, CURRENT_DIR);
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

int main(int const argc, char *argv[]) {
  printf("Welcome to ansh shell, the interactive friendly shell by An\n");
  char cmd[MAX_CMD_LEN];

  if (argc > 1) {
    if (strcmp(argv[1], "-d") == 0) {
      _DEBUG = 1;
    }
  }

  if (argc > 2) {
    if (strcmp(argv[2], "-nf") == 0) {
      _NOFORK = 1;
    }
  }

  // DEBUG MODE
  if (_NOFORK == 1) {
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
      char **paths = malloc(sizeof(char *) * BUFSIZ);
      char *path = cmd + 5;
      parse_input(paths, path);
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
        if (result == -1) {
          fprintf(stderr, "ansh: Unknown command: %s\n", cmd);
          kill(child_pid, SIGTERM);
        }
        printf("\n");
    }
  }

  return 0;
}
