#include "main.h"
#include "parse.h"
#include "utils.h"
#include <errno.h>
#include <limits.h>
#include <pthread.h>
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

int main(int const argc, char *argv[]) {
  // _VERBOSE = 1;
  if (argc > 1) {
    if (strcmp(argv[1], "-d") == 0) {
      _VERBOSE = 2;
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
    _BATCH_MODE = 1;
    DEBUG(argv[1]);
    file = fopen(argv[1], "r");
    if (file == NULL) {
      char current_dir[PATH_MAX];
      if (getcwd(current_dir, PATH_MAX) == NULL) {
        fprintf(stderr, "get directory failed\n");
      }
      snprintf(current_dir, BUFSIZ, "%s/%s", current_dir, argv[1]);
      DEBUG(current_dir);
      file = fopen(current_dir, "r");
      if (file == NULL) {
        print_error_message();
        exit(EXIT_FAILURE);
      }
    }
  }

  int loop_count = 0;
  for (;;) {
    char input[MAX_CMD_LEN];
    if (_INTERACTIVE_MODE == 1) {
      print_prompt();
    }
    fflush(stdout);

    if (fgets(input, MAX_CMD_LEN, file) == NULL) {
      /* file is empty */
      if (loop_count == 0) {
        print_error_message();
        exit(EXIT_FAILURE);
      }
      break;
    }
    DEBUG(input);

    const int input_len = strlen(input);

    /* skip empty command */
    if (input_len == 1) {
      continue;
    }

    char **split_cmds = calloc(strlen(input), sizeof(char **));
    if (split_cmds == NULL) {
      errExit("calloc");
    }

    for (int i = 0; i < input_len; i++) {
      split_cmds[i] = calloc(strlen(input), sizeof(char));
      if (split_cmds[i] == NULL) {
        errExit("calloc");
      }
    }
    const int cmd_counts = split_parallel_cmd(split_cmds, input);

    char debug[20];
    sprintf(debug, "command counts: %d\n", cmd_counts);
    DEBUG(debug);

    pthread_t threads[cmd_counts];
    for (int i = 0; i < cmd_counts; i++) {
      pthread_create(&threads[i], NULL, parse_execute, split_cmds[i]);
    }

    for (int i = 0; i < cmd_counts; i++) {
      if (pthread_join(threads[i], NULL) != 0) {
        errExit("pthread_join");
      }
    }
    loop_count++;
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
    printf(GRN "ansh-> %s" RESET, "");
  }

  free(dirs);
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

  return -1;
}

int execute_command(char *tokens[], const int is_redirect, char out_file[]) {
  // count tokens
  size_t numTokens = 0;
  while (tokens[numTokens] != NULL) {
    numTokens++;
  }

  /*
  =============================
  ===START: BUILT-IN COMMAND===
  =============================
  */
  if (strncmp(tokens[0], "exit", 4) == 0) {
    if (tokens[1] != NULL) {
      print_error_message();
      return 0;
    }
    exit(EXIT_SUCCESS);
  }

  if (strncmp(tokens[0], "cd", 2) == 0) {
    char *dir = tokens[1];
    if (tokens[2] != NULL) {
      print_error_message();
      return 0;
    }

    if (dir == NULL) {
      print_error_message();
      return 0;
    }

    // if (dir == NULL) {
    //   dir = getenv("HOME");
    // }

    if (chdir(trim(dir)) == -1) {
      char current_dir[PATH_MAX];
      if (getcwd(current_dir, PATH_MAX) == NULL) {
        fprintf(stderr, "get directory failed\n");
      }
      snprintf(current_dir, BUFSIZ, "%s/%s", trim(current_dir), dir);
      if (chdir(current_dir) == -1) {
        print_error_message();
        return 0;
      }
    } else {
      return 0;
    }
  }

  if (strncmp(tokens[0], "path", 4) == 0) {
    if (tokens[1] == NULL) {
      SHELL_PATH[0] = NULL;
      return 0;
    }
    SHELL_PATH[0] = NULL;
    size_t i = 0;
    for (i = 0; i < numTokens - 1; i++)
      SHELL_PATH[i] = strdup(tokens[i + 1]);
    SHELL_PATH[i] = NULL;

    return 0;
  }

  /*
  =================================
  =====SETUP SIGNAL HANDLER========
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
  ======================================
  ===START: fork() and exec() command===
  ======================================
  */
  char path[BUFSIZ];
  if (search_path(path, tokens[0]) == -1) {
    print_error_message();
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

    FILE *out = NULL;
    if (is_redirect == 1) {
      out = fopen(out_file, "w+");
      if (out == NULL) {
        errExit("fopen");
      }
      redirect(out);
    }

    execv(path, tokens);
    fprintf(stderr, "ansh: Unknown command: %s\n", tokens[0]);
    _exit(127);
  default:
    while (waitpid(child, &status, 0) == -1) {
      return -1;
    }
  /*
    system calls may report error code EINTR if
    a signal occured while system call was inprogress
    -> no error actually occurred -> retries waitpid()
    */
    if (errno != EINTR) {
      status = -1;
      break;
    }
    DEBUG("child done");

    break;
  }

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
void redirect(FILE *out) {
  // Flush the output stream to ensure no data is lost during redirection
  fflush(stdout);

  const int output_fd = fileno(out);
  if (output_fd != STDOUT_FILENO) {
    if (dup2(output_fd, STDOUT_FILENO) == -1) {
      errExit("dup2 2");
    }
    if (fclose(out) == -1) {
      errExit("close");
    }
  }
}


void *parse_execute(void *ptr) {
  int is_redirect = 0;

  const char *input = (char *)ptr;
  const int input_len = strlen(input);
  if (input_len == 0) {
    return NULL;
  }
  // trim input

  char output_file[input_len];
  char cmd[input_len];
  const int split_status = split_output(cmd, output_file, input);
  if (split_status == 1) {
    is_redirect = 1;
  } else if (split_status == -1) {
    fprintf(stderr, "An error has occurred\n");
    return NULL;
  }
  const int cmd_len = strlen(input);

  char **tokens = calloc(cmd_len, sizeof(char **));
  if (tokens == NULL) {
    errExit("calloc");
  }
  for (int i = 0; i < cmd_len; i++) {
    tokens[i] = calloc(cmd_len, sizeof(char));
    if (tokens[i] == NULL) {
      errExit("calloc");
    }
  }
  parse_inputv2(tokens, cmd);
  execute_command(tokens, is_redirect, output_file);

  for (int j = 0; j < cmd_len; j++) {
    free(tokens[j]);
  }
  free(tokens);

  return NULL;
}
