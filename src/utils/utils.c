#include "../include/globals.h"
#include "./utils.h"

void list_dir(const char *path, const char *filter)
{
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        perror("opendir");
        return;
    }

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type != DT_REG)
            continue;

        if (filter != NULL && strstr(entry->d_name, filter) == NULL)
            continue;

        char name[strlen(entry->d_name) + 1];
        strcpy(name, entry->d_name);

        // Remove file extension
        char *dot = strrchr(name, '.');
        if (dot != NULL)
            *dot = '\0';

        print(name, NULL);
    }

    closedir(dir);
}

void print(const char *code, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    if (msg == NULL || msg[0] == '\0')
    {
        va_end(args);
        return;
    }

    if (code == NULL || code[0] == '\0')
    {
        printf("%s\n", msg);
        vprintf(msg, args);
        errno == 0 ? printf("") : printf(" (%s)", strerror(errno));

        va_end(args);
        return;
    }

    const char *color = "";

    if (strcmp(code, WARNING) == 0)
        color = YELLOW;
    else if (strcmp(code, ERROR) == 0)
        color = RED;
    else if (strcmp(code, CAUTION) == 0)
        color = BLUE;
    else if (strcmp(code, SUCCESS) == 0)
        color = GREEN;

    if (color[0] != '\0')
    {
        printf("%s%s[%s]%s ", color, BOLD, code, RESET);
        vprintf(msg, args);
        errno == 0 ? printf("") : printf(" (%s)", strerror(errno));
        printf("\n");
    }
    else
    {
        printf("%s\n", msg);
        vprintf(msg, args);
        errno == 0 ? printf("") : printf(" (%s)", strerror(errno));
    }

    va_end(args);
    return;
}