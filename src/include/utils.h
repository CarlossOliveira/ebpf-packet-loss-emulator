#ifndef UTILS_H
#define UTILS_H

#ifdef APP
#include <stddef.h>
#include <stdint.h>

#define WARNING "WARNING"
#define ERROR "ERROR"
#define CAUTION "CAUTION"
#define SUCCESS "SUCCESS"

void list_dir(const char *path, const char *filter);
void print(const char *code, const char *msg, ...);
int input(char *prompt, char *buffer, size_t size);
void lower(char *str);
int count_char(const char *str, char c);
void remove_char(char *str, char c);
char **strsplit(const char *str, char delim);
void strsplit_free(char **parts);
uint64_t read_stats_map(uint8_t key);
int dump_to_log_file(const char *filename, const char *data);
int dump_stats(void);
char *get_current_time(char *buffer, size_t size);
int check_sudo(void);
void wait_for_char(char expected_char);
#endif // !APP

#endif // !UTILS_H