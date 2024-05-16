#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int extract_quoted_string(char *str, char *buffer, size_t buffer_size) {
  char *start = strchr(str, '\"');
  char *end = strrchr(str, '\"');

  if (start && end && end > start) {
    size_t len = end - start;
    if (len - 1 < buffer_size) {
      strncpy(buffer, start + 1, len - 1);
      buffer[len - 1] = '\0';
      return 0;
    } else {
      return -1; // Buffer too small
    }
  }

  return -1; // No quoted string found
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
