#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <stdatomic.h>

#define WARNING "WARNING"
#define ERROR "ERROR"
#define CAUTION "CAUTION"
#define SUCCESS "SUCCESS"

#define RED "\033[31m"
#define YELLOW "\033[33m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define BOLD "\033[1m"
#define RESET "\033[0m"

void print(const char *code, const char *msg, ...);
void wait_for_char(char expected_char, atomic_bool *active);

#endif // !IO_UTILS_H