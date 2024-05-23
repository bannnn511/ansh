#include "utils.h"
#include <ctype.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int split_parallel_cmd(char **cmds, const char *input) {
  regex_t regex;
  const char *pattern = "[^&]+";
  if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
    fprintf(stderr, "could not compile regex");
    return -1;
  }

  regmatch_t pmatch[strlen(input)];
  int i;
  for (i = 0;; i++) {
    if (regexec(&regex, input, strlen(input), pmatch, 0))
      break;
    const int start = pmatch[0].rm_so;
    const int end = pmatch[0].rm_eo;
    char buffer[end - start];
    strncpy(buffer, input + start, end - start);
    buffer[end - start] = '\0';
    strncpy(cmds[i], trim(buffer), strlen(buffer) + 1);
    input += end;
  }
  cmds[i] = NULL;

  regfree(&regex);

  return i;
}

int split_output(char *cmd, char *file_name, const char *input) {
  regex_t regex;
  const char *pattern = "[^>]+";

  if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
    fprintf(stderr, "could not compile regex");
    return -1;
  }

  regmatch_t pmatch[2];
  if (regexec(&regex, input, 2, pmatch, 0) == 0) {
    const int start = pmatch[0].rm_so;
    const int end = pmatch[0].rm_eo;
    char buffer[end - start];
    strncpy(buffer, input + start, end - start);
    strncpy(cmd, trim(buffer), strlen(buffer));
    cmd[end - start] = '\0';
    input += end;
  } else {
    return -1;
  }

  if (regexec(&regex, input, 2, pmatch, 0) == 0) {
    const int start = pmatch[0].rm_so;
    const int end = pmatch[0].rm_eo;
    char buffer[end - start];
    strncpy(buffer, input + start + 1, end - start + 1);
    buffer[end - start] = '\0';
    strncpy(file_name, trim(buffer), strlen(buffer) + 1);
  } else {
    regfree(&regex);
    return 0;
  }

  regfree(&regex);

  return 1;
}

int parse_inputv2(char **buffers, const char *input) {
  regex_t regex;
  const char *pattern = "[^ ]+";
  if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
    fprintf(stderr, "could not compile regex");
    return -1;
  }

  regmatch_t pmatch[strlen(input)];
  int i;
  for (i = 0;; i++) {
    if (regexec(&regex, input, strlen(input), pmatch, 0))
      break;
    const int start = pmatch[0].rm_so;
    const int end = pmatch[0].rm_eo;
    char buffer[end - start];
    strncpy(buffer, input + start, end - start);
    buffer[end - start] = '\0';
    strncpy(buffers[i], trim(buffer), strlen(buffer) + 1);
    input += end;
  }
  buffers[i] = NULL;

  regfree(&regex);

  return i;
}
