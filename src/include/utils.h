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
void lower(char *str);
int write_to_map(int map_fd, uint32_t key, uint64_t value);
uint64_t read_from_map(int map_fd, uint32_t key);
int dump_to_log_file(const char *filename, const char *data);
int dump_stats_to_log_file(void);
char *get_current_time(char *buffer, size_t size);
#endif // !APP

#ifdef BPF
long update_stats(__u32 key, __u64 value);
__u64 get_stats(__u32 key);
#endif // !BPF

#endif // !UTILS_H