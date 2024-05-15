#include <ctype.h>
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

int _DEBUG = 0;
int _NOFORK = 0;

char* SHELL_PATH[BUFSIZ] = {"/bin/", "/usr/bin/", NULL};

char* trim(char* s)
{
  while (isspace(*s))
    s++;

  if (*s == '\0')
  {
    return s;
  }

  char* end = s + strlen(s) - 1;
  while (end > s && isspace(*end))
    end--;

  end[1] = '\0';

  return s;
}

void print_debug(char* msg)
{
  if (_DEBUG == 0)
  {
    return;
  }
  printf("ansh(debug)-> %s\n", msg);
}

int parse_input(char** tokens, char* cmd)
{
  char* token;
  int i = 0;
  while ((token = strsep(&cmd, " ")) != NULL)
  {
    if (*token == '\0')
    {
      continue;
    }
    tokens[i] = trim(token);
    i++;
  }

  return i;
}

int exec_child(char* cmd)
{
  char** tokens = malloc(sizeof(char*) * BUFSIZ);
  parse_input(tokens, cmd);

  char** path;
  char* base_cmd = tokens[0];
  for (path = SHELL_PATH; *path; path++)
  {
    char* cmd_path = malloc(strlen(*path) + strlen(base_cmd) + 1);
    if (cmd_path == NULL)
    {
      errExit("malloc");
    }
    strcpy(cmd_path, *path);
    strcat(cmd_path, base_cmd);

    // check if path
    if (access(cmd_path, X_OK) == -1)
    {
      print_debug(cmd_path);
      free(cmd_path);
      continue;
    }

    execv(cmd_path, tokens);
    perror("exec_child");
    free(cmd_path);
    return -1;
  }

  fprintf(stderr, "%s: command not found\n", base_cmd);
  return -1;
}

int main(const int argc, char* argv[])
{
  printf("Welcome to ansh shell, the interactive friendly shell by An\n");
  char cmd[MAX_CMD_LEN];

  if (argc > 1)
  {
    if (strcmp(argv[1], "-d") == 0)
    {
      _DEBUG = 1;
    }
  }

  if (argc > 2)
  {
    if (strcmp(argv[2], "-nf") == 0)
    {
      _NOFORK = 1;
    }
  }

  // DEBUG MODE
  if (_NOFORK == 1)
  {
    if (fgets(cmd, MAX_CMD_LEN, stdin) == NULL)
    {
      return -1;
    }
    exec_child(cmd);
    return 0;
  }

  for (;;)
  {
    printf("ansh-> ");
    fflush(stdout);

    if (fgets(cmd, MAX_CMD_LEN, stdin) == NULL)
    {
      break;
    }

    if (strncmp(cmd, "exit", 4) == 0)
    {
      return 0;
    }

    if (strncmp(cmd, "cd", 2) == 0)
    {
      char* path = cmd + 3;
      if (chdir(trim(path)) == -1)
      {
        perror("chdir");
      }
      continue;
    }

    pid_t child;
    switch (child = fork())
    {
    case -1:
      errExit("fork");
    case 0:
      exec_child(cmd);
    default:
      if (waitpid(child, NULL, 0) == -1)
      {
        errExit("waitpid");
      }
    }
  }

  return 0;
}

