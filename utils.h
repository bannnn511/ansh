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

/* Debug mode will print prompt to command line */
extern int _DEBUG;

void print_simple_prompt(void);
void print_debug(char* msg);

#endif //UTILS_H
