#include "list.h"

#include "globals.h"

#include "commands.h"
#include "io_utils.h"

#include <dirent.h>
#include <string.h>

static int list_dir(const char *path, const char *filter);

void list_modules_command(char **input)
{
	if (input)
		if (HELP_FLAG) {
			print(NULL, LIST_HELP_MESSAGE);
			return;
		}

	print(NULL, "Available eBPF modules:");
	if (list_dir(BPF_OBJECT_DIR, ".bpf.o") != 0) {
		print(ERROR, "Failed to list BPF modules");
		return;
	}
}

static int list_dir(const char *path, const char *filter)
{
	if (!path) {
		print(ERROR, "Path is required to list directory");
		return 1;
	}

	DIR *dir = opendir(path);
	if (!dir) {
		print(ERROR, "Failed to open directory %s", path);
		return 1;
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
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
