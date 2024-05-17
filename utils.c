#include <stdio.h>
#include "utils.h"

int _DEBUG = 0;

void print_simple_prompt(void) { printf(GRN "ansh-> " RESET); }

void print_debug(char *msg) {
  if (_DEBUG == 0) {
    return;
  }
  printf("ansh(debug)-> %s", msg);
}

