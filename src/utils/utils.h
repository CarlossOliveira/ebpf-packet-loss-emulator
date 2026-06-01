#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>
#include <errno.h>

#define RED "\033[31m"
#define YELLOW "\033[33m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"

#define BOLD "\033[1m"
#define RESET "\033[0m"

#define WARNING "WARNING"
#define ERROR "ERROR"
#define CAUTION "CAUTION"
#define SUCCESS "SUCCESS"

void print(const char *code, const char *msg, ...);
void list_dir(const char *path, const char *filter);

#endif // UTILS_H