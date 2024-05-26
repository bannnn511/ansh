#ifndef UTILS_H
#define UTILS_H

#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))


/* Debug mode will print prompt to command line */
extern int _VERBOSE;
#define SHOW_ERROR 1
#define SHOW_WARNING 2

#define DEBUG(...)\
if(_VERBOSE && SHOW_ERROR) {\
printf("Error : %s, %d ",__FUNCTION__, __LINE__);\
printf("%s",__VA_ARGS__);\
}\
else if (_VERBOSE && SHOW_WARNING) {\
printf("Warning : %s, %d ",__FUNCTION__, __LINE__);\
printf("%s",__VA_ARGS__);\
}

void print_simple_prompt(void);
void print_error_message(void);

#endif //UTILS_H
