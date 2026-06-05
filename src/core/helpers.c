#include "globals.h"

#include "io_utils.h"
#include "time_utils.h"

#include <bpf/bpf.h>
#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int input(char *prompt, char *buffer, size_t size) {
  if (!buffer || size == 0)
    return 1;

  fflush(stdout); // Ensure prompt is printed before input
  char *line = readline(prompt ? prompt : "");

  if (!atomic_load(&active) || atomic_load(&bpf_module_change_requested)) {
    free(line);
    return 1;
  }

  if (!line) {
    atomic_store(&active, false);
    return 1;
  }

  if (line[0] != '\0')
    add_history(line);

  snprintf(buffer, size, "%s", line);
  free(line);
  return 0;
}

int list_dir(const char *path, const char *filter) {
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

uint64_t read_stats_map(uint8_t key) {
  uint64_t value = 0;

  if (stats_map_fd < 0)
    return 0;

  if (bpf_map_lookup_elem(stats_map_fd, &key, &value) != 0)
    return 0;

  return value;
}

int dump_to_log_file(const char *filename, const char *data) {
  FILE *file = fopen(filename, "a");

  if (!file) {
    // Extract directory from filename
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "%s", filename);

    // Create directory if it doesn't exist
    char *dir = dirname(path);
    if (mkdir(dir, 0755) != 0 && errno != EEXIST) {
      print(ERROR, "Failed to create log directory");
      return 1;
    }

    // Try opening the file again
    file = fopen(filename, "a");

    if (!file) {
      print(ERROR, "Failed to open log file");
      return 1;
    }
  }

  fprintf(file, "%s\n", data);
  fclose(file);

  return 0;
}

int dump_stats() {
  if (stats_map_fd > 0) {
    char stats[512];
    char time_buffer[20];

    snprintf(stats, sizeof(stats),
             "---------- STATS ----------\n[%s]\nBPF Module: %s\nPackets "
             "processed: %lu\nPackets dropped: %lu\nTotal execution time: %lu "
             "ns\n---------------------------",
             get_current_time(time_buffer, sizeof(time_buffer)),
             bpf_module_name, (unsigned long)read_stats_map(PACKETS_PROCESSED),
             (unsigned long)read_stats_map(PACKETS_DROPPED),
             (unsigned long)read_stats_map(TOTAL_EXECUTION_TIME));

    printf("%s", BLUE);
    print(NULL, "%s", stats);
    printf("%s", RESET);

    return dump_to_log_file(LOG_FILE, stats);
  } else
    return 1;
}
