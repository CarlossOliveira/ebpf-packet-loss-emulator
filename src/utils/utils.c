#include "../include/globals.h"
#include "../include/utils.h"

#include <bpf/bpf.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define RED "\033[31m"
#define YELLOW "\033[33m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define BOLD "\033[1m"
#define RESET "\033[0m"

void list_dir(const char *path, const char *filter)
{
    DIR *dir = opendir(path);
    if (!dir)
    {
        print(ERROR, "Failed to open directory %s", path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type != DT_REG)
            continue;
        if (filter && strstr(entry->d_name, filter) == NULL)
            continue;

        char name[256];
        snprintf(name, sizeof(name), "%s", entry->d_name);
        char *suffix = strstr(name, ".bpf.o");
        if (suffix)
            *suffix = '\0';

        print(NULL, "-> %s", name);
    }

    closedir(dir);
}

void print(const char *code, const char *msg, ...)
{
    if (!msg || !msg[0])
        return;

    const char *color = "";
    if (code)
    {
        if (strcmp(code, WARNING) == 0)
            color = YELLOW;
        else if (strcmp(code, ERROR) == 0)
            color = RED;
        else if (strcmp(code, CAUTION) == 0)
            color = BLUE;
        else if (strcmp(code, SUCCESS) == 0)
            color = GREEN;
    }

    if (code && code[0] && color[0])
        printf("%s%s[%s]%s ", color, BOLD, code, RESET);
    else if (code && code[0])
        printf("[%s] ", code);

    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);

    if (errno != 0 && strcmp(color, RED) == 0)
        printf(" (%s)", strerror(errno));
    printf("\n");
    errno = 0;
}

void lower(char *str)
{
    if (!str)
        return;

    for (size_t i = 0; str[i] != '\0'; i++)
        str[i] = (char)tolower((unsigned char)str[i]);
}

int write_to_map(int map_fd, uint32_t key, uint64_t value)
{
    return bpf_map_update_elem(map_fd, &key, &value, BPF_ANY);
}

uint64_t read_from_map(int map_fd, uint32_t key)
{
    uint64_t value = 0;

    if (map_fd < 0)
        return 0;

    if (bpf_map_lookup_elem(map_fd, &key, &value) != 0)
        return 0;

    return value;
}

int dump_to_log_file(const char *filename, const char *data)
{
    FILE *file = fopen(filename, "a");
    if (!file)
    {
        print(ERROR, "Failed to open log file");
        return 1;
    }

    fprintf(file, "%s\n", data);
    fclose(file);
    return 0;
}

int dump_stats_to_log_file(void)
{
    char buffer[512];
    char time_buffer[20];

    snprintf(buffer, sizeof(buffer),
             "--------------------\n[%s]\nBPF Module: %s\nPackets processed: %lu\nPackets dropped: %lu\nTotal execution time: %lu ns\n--------------------",
             get_current_time(time_buffer, sizeof(time_buffer)),
             bpf_module_name,
             (unsigned long)read_from_map(stats_map_fd, PACKETS_PROCESSED),
             (unsigned long)read_from_map(stats_map_fd, PACKETS_DROPPED),
             (unsigned long)read_from_map(stats_map_fd, TOTAL_EXECUTION_TIME));

    return dump_to_log_file(LOG_FILE, buffer);
}

char *get_current_time(char *buffer, size_t size)
{
    if (!buffer || size == 0)
        return NULL;

    time_t now = time(NULL);
    struct tm t;
    localtime_r(&now, &t);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &t);
    return buffer;
}