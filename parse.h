#ifndef PARSE_H
#define PARSE_H
#include <stddef.h>

int extract_quoted_string(char* str, char* buffer, size_t buffer_size);
char* trim(char* s);
int parse_input(char** tokens, char* cmd);
int parse_dir(char** tokens, char* cmd);
#endif // PARSE_H
