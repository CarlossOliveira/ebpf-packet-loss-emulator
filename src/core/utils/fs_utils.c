#include "fs_utils.h"

#include "io_utils.h" // print()

#include <dirent.h> // DIR, opendir, readdir, closedir, struct dirent
#include <stdio.h>  // snprintf
#include <string.h> // strstr

int list_dir(const char *path, const char *filter)
{
    DIR *dir = opendir(path);
    if (!dir)
    {
        print(ERROR, "Failed to open directory %s", path);
        return 1;
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
    return 0;
}
