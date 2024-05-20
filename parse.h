#ifndef PARSE_H
#define PARSE_H

int extract_quoted_string(char* buffer, const char* cmd);
char* trim(char* s);
int split_line(char** tokens, char* cmd, const char* delim);
int parse_input(char** buffers, const char* cmd);
int extract_output_file(char *file_name, const char *cmd);

#endif // PARSE_H
