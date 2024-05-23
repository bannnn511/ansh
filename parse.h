#ifndef PARSE_H
#define PARSE_H

int extract_quoted_string(char* buffer, const char* cmd);
char* trim(char* s);
int split_line(char** tokens, char* cmd, const char* delim);
int parse_input(char** buffers, const char* cmd);
int parse_inputv2(char** buffers, const char* cmd);
int split_output(char cmd[],char file_name[], const char* input);
#endif // PARSE_H
