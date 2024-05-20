#include "utils.h"
#include <ctype.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int extract_quoted_string(char *buffer, const char *str) {
  regex_t regex;
  const char *pattern = "\"[^\"]*";

  if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
    fprintf(stderr, "could not compile regex");
    return -1;
  }

  regmatch_t pmatch[2];
  if (regexec(&regex, str, 2, pmatch, 0) == 0) {
    const int start = pmatch[0].rm_so;
    const int end = pmatch[0].rm_eo;
    strncpy(buffer, str + start, end - start);
  } else {
    return -1;
  }

  regfree(&regex);

  return 0;
}

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

int split_line(char **tokens, char *cmd, const char *delim) {
  char *token;
  int i = 0;
  while ((token = strsep(&cmd, delim)) != NULL) {
    if (*token == '\0') {
      continue;
      ;
    }
    tokens[i] = trim(token);
    i++;
  }

  return i - 1;
}

int parse_input(char **buffers, const char *cmd) {
  // copy command
  char *cmd_copy = calloc(strlen(cmd), sizeof(char));
  if (cmd_copy == NULL) {
    return -1;
  }
  strcpy(cmd_copy, cmd);

  // split command by quotes
  char **commands = calloc(BUFSIZ, sizeof(char *));
  split_line(commands, cmd_copy, "\"");

  // extract arguments
  char *args = calloc(BUFSIZ, sizeof(char *));
  if (commands[1] != NULL && strlen(commands[1]) > 0) {
    if (extract_quoted_string(args, cmd) != 0) {
      char buffer[20];
      sprintf(buffer, "%s\n", commands[1]);
      print_debug(cmd_copy);

      return -1;
    }
  }

  char **tokens = calloc(BUFSIZ, sizeof(char *));
  int i = split_line(tokens, commands[0], " ");

  // copy tokens to buffers
  for (int j = 0; j <= i; j++) {
    strncpy(buffers[j], tokens[j], strlen(tokens[j]));
  }
  // copy args to buffers if not empty
  if (strncmp(args, "", 1) != 0) {
    strncpy(buffers[++i], args, strlen(args));
  }
  buffers[++i] = NULL;

  free(tokens);
  free(args);
  free(cmd_copy);

  return i;
}

int extract_output_file(char *file_name, const char *cmd) {
  regex_t regex;
  const char *pattern = ">([^>]+)";

  if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
    fprintf(stderr, "could not compile regex");
    return -1;
  }

  regmatch_t pmatch[2];
  if (regexec(&regex, cmd, 2, pmatch, 0) == 0) {
    const int start = pmatch[0].rm_so;
    const int end = pmatch[0].rm_eo;
    char buffer[end - start];
    strncpy(buffer, cmd + start + 1, end - start + 1);
    buffer[end - start] = '\0';
    strncpy(file_name, trim(buffer), strlen(buffer) + 1);
  } else {
    return -1;
  }

  regfree(&regex);

  return 0;
}
