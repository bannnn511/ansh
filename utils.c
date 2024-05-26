#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int _VERBOSE = 0;
static char error_message[30] = "An error has occurred\n";

void print_simple_prompt(void) { printf(GRN "ansh-> " RESET); }

void print_error_message(void) {
  write(STDERR_FILENO, error_message, strlen(error_message));
}