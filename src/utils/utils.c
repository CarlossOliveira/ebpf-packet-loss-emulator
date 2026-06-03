#include "../include/globals.h"
#include "../include/utils.h"

#include <bpf/bpf.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

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

    if (errno != 0 && code && strcmp(color, RED) == 0) // Only print the error string for ERROR messages
        printf(" (%s)", strerror(errno));
    printf("\n");
    errno = 0;
}

int input(char *prompt, char *buffer, size_t size)
{
    if (!buffer || size == 0)
        return 1;

    char *line = readline(prompt ? prompt : "");

    if (!atomic_load(&active) || atomic_load(&bpf_module_change_requested))
    {
        free(line);
        return 1;
    }

    if (!line)
        return -1;

    if (line[0] != '\0')
        add_history(line);

    snprintf(buffer, size, "%s", line);
    free(line);
    return 0;
}

void lower(char *str)
{
    if (!str)
        return;

    for (size_t i = 0; str[i] != '\0'; i++)
        str[i] = (char)tolower((unsigned char)str[i]);
}

int count_char(const char *str, char c)
{
    int count = 0;

    if (!str)
        return 0;

    while (*str)
    {
        if (*str == c)
            count++;
        str++;
    }

    return count;
}

void remove_char(char *str, char c)
{
    if (!str)
        return;

    char *src = str, *dst = str;

    while (*src)
    {
        if (*src != c)
            *dst++ = *src;
        src++;
    }
    *dst = '\0';
}

char **strsplit(const char *str, char delim)
{
    if (!str)
        return NULL;

    int count = count_char(str, delim) + 1;
    char **result = calloc((size_t)count + 1, sizeof(char *));
    if (!result)
        return NULL;

    char *copy = strdup(str);
    if (!copy)
    {
        free(result);
        return NULL;
    }

    char delim_str[2] = {delim, '\0'};

    int index = 0;
    char *token = strtok(copy, delim_str);

    while (token)
    {
        result[index] = strdup(token);
        if (!result[index])
        {
            strsplit_free(result);
            free(copy);
            return NULL;
        }
        index++;
        token = strtok(NULL, delim_str);
    }

    result[index] = NULL;
    free(copy);
    return result;
}

void strsplit_free(char **parts)
{
    if (!parts)
        return;

    for (size_t i = 0; parts[i] != NULL; i++)
        free(parts[i]);
    free(parts);
}

uint64_t read_stats_map(uint8_t key)
{
    uint64_t value = 0;

    if (stats_map_fd < 0)
        return 0;

    if (bpf_map_lookup_elem(stats_map_fd, &key, &value) != 0)
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

int dump_stats()
{
    if (stats_map_fd > 0)
    {
        char stats[512];
        char time_buffer[20];

        snprintf(stats, sizeof(stats),
                 "---------- STATS ----------\n[%s]\nBPF Module: %s\nPackets processed: %lu\nPackets dropped: %lu\nTotal execution time: %lu ns\n---------------------------",
                 get_current_time(time_buffer, sizeof(time_buffer)),
                 bpf_module_name,
                 (unsigned long)read_stats_map(PACKETS_PROCESSED),
                 (unsigned long)read_stats_map(PACKETS_DROPPED),
                 (unsigned long)read_stats_map(TOTAL_EXECUTION_TIME));

        printf("%s", BLUE);
        print(NULL, "%s", stats);
        printf("%s", RESET);

        return dump_to_log_file(LOG_FILE, stats);
    }
    else
        return 1;
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

int check_sudo(void)
{
    if (geteuid() != 0)
    {
        print(ERROR, "This program must be run as root.");
        return 1;
    }
    return 0;
}

void wait_for_char(char expected_char)
{
    int c;

    while (1)
    {
        errno = 0;
        c = getchar();

        if (c == expected_char)
            return;

        if (c == EOF)
        {
            if (errno == EINTR && atomic_load(&active))
            {
                clearerr(stdin);
                continue;
            }
            if (!atomic_load(&active))
                return;

            clearerr(stdin);
            continue;
        }
    }
}