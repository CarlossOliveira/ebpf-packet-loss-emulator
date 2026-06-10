#ifndef STRING_UTILS_H
#define STRING_UTILS_H

void lower(char *str);

int count_char(const char *str, char c);

void remove_char(char *str, char c);

char **strsplit(const char *str, char delim, int max_parts);

void strsplit_free(char **parts);

#endif // !STRING_UTILS_H
