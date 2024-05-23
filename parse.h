#ifndef PARSE_H
#define PARSE_H

char* trim(char* s);
int split_line(char** tokens, char* cmd, const char* delim);
int parse_inputv2(char** buffers, const char* cmd);
int split_output(char cmd[],char file_name[], const char* input);
int split_parallel_cmd(char **cmds, const char *input);

#endif // PARSE_H
