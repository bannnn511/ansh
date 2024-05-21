#include "parse.h"
#include "utils.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __unix__
#include <sys/types.h>
#include <sys/wait.h>
#define PATH_MAX 1024 /* max bytes in pathname */
#endif

#define MAX_CMD_LEN 200

#define errExit(msg)                                                           \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

/* Shell will be executed in interactive mode */
int _INTERACTIVE_MODE = 1;

/* Array of paths where the shell should look for commands */
char *SHELL_PATH[BUFSIZ] = {"/bin", "/usr/bin", "/opt/homebrew/bin", NULL};

void print_prompt(void);
void print_simple_prompt(void);
void print_debug(char *msg);
void update_path(char **paths);
int execute_command(char *tokens[]);
void redirect(FILE *out, FILE *temp);

int main(int const argc, char *argv[]) {
  if (argc > 1) {
    if (strcmp(argv[1], "-d") == 0) {
      _DEBUG = 1;
    } else {
      _INTERACTIVE_MODE = 0;
    }
  }

  if (_INTERACTIVE_MODE == 1) {
    printf("Welcome to ansh shell, the interactive friendly shell by An\n");
  }

  /* file for batch mode */
  FILE *file;
  if (_INTERACTIVE_MODE) {
    file = stdin;
  } else {
    file = fopen(argv[1], "r");
    if (file == NULL) {
      fprintf(stderr, "error: No such file or directory");
    }
  }

  /* file for output redirection */
  FILE *out = NULL;
  int is_redirect = 0;

  for (;;) {
    char cmd[MAX_CMD_LEN];
    if (_INTERACTIVE_MODE == 1) {
      print_prompt();
    }
    fflush(stdout);

    if (fgets(cmd, MAX_CMD_LEN, file) == NULL) {
      break;
    }
    print_debug(cmd);

    /* skip empty command */
    if (strlen(cmd) == 1) {
      continue;
    }

    /* remove new line */
    if (cmd[strlen(cmd) - 1] == '\n') {
      cmd[strlen(cmd) - 1] = '\0';
    }

    char output_file[BUFSIZ];
    FILE *temp_fd = tmpfile();
    if (extract_output_file(output_file, cmd) == 0) {
      is_redirect = 1;
      out = fopen(output_file, "w");
      redirect(out, temp_fd);
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
    parse_inputv2(tokens, cmd);
    execute_command(tokens);

    for (unsigned long j = 0; j < strlen(cmd); j++) {
      free(tokens[j]);
    }
    free(tokens);

    /* redirect back to STDOUT after executing command */
    if (is_redirect == 1) {
      is_redirect = 0;
      if (dup2(fileno(temp_fd), STDOUT_FILENO) == -1) {
        errExit("dup2 3");
      }
    }
  }

  if (out != NULL) {
    fclose(out);
  }

  if (_INTERACTIVE_MODE == 0) {
    fclose(file);
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

  if (getcwd(current_dir, PATH_MAX) == NULL) {
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
  /*
  =============================
  ===START: BUILT-IN COMMAND===
  =============================
  */
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
  /*
  ===========================
  ===END: BUILT-IN COMMAND===
  ===========================
  */

  /*
  =================================
  ===START: SETUP SIGNAL HANDLER===
  =================================
  */
  sigset_t blockMask, origMask;
  struct sigaction saIgnore, saOrigQuit, saOrigInt, saDefault;

  sigemptyset(&blockMask); /* Block SIGCHLD */
  sigaddset(&blockMask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &blockMask, &origMask);

  saIgnore.sa_handler = SIG_IGN; /* ignore SIGINT and SIGQUIT */
  saIgnore.sa_flags = 0;
  sigemptyset(&saIgnore.sa_mask);
  sigaction(SIGINT, &saIgnore, &saOrigInt);
  sigaction(SIGQUIT, &saIgnore, &saOrigQuit);
  /*
  =================================
  ===END: SETUP SIGNAL HANDLER===
  =================================
  */

  /*
  ======================================
  ===START: fork() and exec() command===
  ======================================
  */
  char path[BUFSIZ];
  if (search_path(path, tokens[0]) == -1) {
    return -1;
  }

  pid_t child;
  int status;
  switch (child = fork()) {
  case -1:
    errExit("fork");
  case 0:
    saDefault.sa_handler = SIG_DFL;
    saDefault.sa_flags = 0;
    sigemptyset(&saDefault.sa_mask);

  /* resets the dispositions to SIG_DFL */
    if (saOrigInt.sa_handler != SIG_IGN) {
      sigaction(SIGINT, &saDefault, NULL);
    }
    if (saOrigQuit.sa_handler != SIG_IGN) {
      sigaction(SIGQUIT, &saDefault, NULL);
    }

    sigprocmask(SIG_SETMASK, &origMask, NULL);

    execv(path, tokens);
    fprintf(stderr, "ansh: Unknown command: %s\n", tokens[0]);
    _exit(127);
  default:
    while (waitpid(child, &status, 0) == -1) {
      return -1;
    }
  /*
    system calls my report error code EINTR if a signal occured while system
    call was inprogress
    -> no error actually occurred -> retries the system call
    */
    if (errno != EINTR) {
      status = -1;
      break;
    }
    print_debug("child done\n");

    break;
  }
  /*
  ======================================
  ===END: fork() and exec() command===
  ======================================
  */

  /* Unblock SIGCHLD, restore dispositions of SIGINT and SIGQUIT */
  const int savedErrno = errno;

  sigprocmask(SIG_SETMASK, &origMask, NULL);
  sigaction(SIGINT, &saOrigInt, NULL);
  sigaction(SIGQUIT, &saOrigQuit, NULL);

  errno = savedErrno;

  return status;
}

/*
  redirect will duplicate fd of STDOUT to out. out will be close after
  sucessfull called to dup2 if temp is not null -> duplicate STDOUNT to temp for
  later reuse
*/
void redirect(FILE *out, FILE *temp) {
  if (temp != NULL) {
    if (dup2(STDOUT_FILENO, fileno(temp)) == -1) {
      errExit("dup2 1");
    }
  }

  int output_fd = fileno(out);
  if (output_fd != STDOUT_FILENO) {
    if (dup2(output_fd, STDOUT_FILENO) == -1) {
      errExit("dup2 2");
    }
    close(output_fd);
  }
}
