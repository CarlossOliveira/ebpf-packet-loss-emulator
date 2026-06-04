#include "globals.h"

#include "fs_utils.h"
#include "io_utils.h"
#include "privilege_utils.h"
#include "string_utils.h"

#include <bpf/bpf.h>
#include <errno.h>
#include <net/if.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define HELP_COMMAND "help"
#define STATS_COMMAND "stats"
#define CHANGE_CONFIG_COMMAND "config"
#define CHANGE_MODULE_COMMAND "change"
#define CLEAR_SCREEN_COMMAND "clear"
#define EXIT_COMMAND "exit"

static void parse_command(char *input);

int main(int argc, char *argv[]) {
#ifndef DEBUG
  libbpf_set_print(NULL);
#endif // !DEBUG

  if (require_root() != 0)
    return 1;

  if (argc < 2 || argc > 3) {
    print(ERROR, "Usage: %s <interface> [<attach_point>]", argv[0]);
    return 1;
  }

  if (if_nametoindex(argv[1]) == 0) {
    print(ERROR, "Invalid interface: %s", argv[1]);
    return 1;
  }

  interface = argv[1];

  if (argc == 3) {
    lower(argv[2]);
    if (strcmp(argv[2], "egress") == 0)
      attach_point = BPF_TC_EGRESS;
    else if (strcmp(argv[2], "ingress") == 0)
      attach_point = BPF_TC_INGRESS;
    else if ((strcmp(argv[2], "ingress|egress") == 0) ||
             (strcmp(argv[2], "egress|ingress") == 0))
      attach_point = BPF_TC_INGRESS | BPF_TC_EGRESS;
    else {
      print(ERROR,
            "Invalid attach point: %s. Use 'ingress', 'egress', or "
            "'ingress|egress'.",
            argv[2]);
      return 1;
    }
  }

  if (setup() != 0)
    return 1;

  while (atomic_load(&active)) {
    print(NULL, "Press Ctrl+C to exit, Ctrl+\\ to change module.");
    print(NULL, "Available eBPF modules:");
    if (list_dir(BPF_MODULES_DIR, ".bpf.o") != 0) {
      print(ERROR, "Failed to list BPF modules");
      return 1;
    }

    char choice[256] = {0};
    int input_result = input(
        "Enter the name of the eBPF module to load: ", choice, sizeof(choice));
    if (input_result == -1)
      continue;
    else if (input_result == 1)
      goto unmount;

    if (choice[0] == '\0')
      continue;

    snprintf(bpf_module_name, sizeof(bpf_module_name), "%s", choice);
    bpf_module_name[sizeof(bpf_module_name) - 1] = '\0';

    bpf_loaded_program_obj = mount_bpf_module(choice, interface);
    if (bpf_loaded_program_obj == NULL) {
      print(ERROR, "Failed to attach module: %s", choice);
      continue;
    }
    print(SUCCESS, "Module '%s' attached successfully", choice);

    while (!atomic_load(&bpf_module_change_requested) && atomic_load(&active)) {
      char cmd[256] = {0};
      int cmd_result = input("> ", cmd, sizeof(cmd));
      if (cmd_result == -1)
        continue;
      else if (cmd_result == 1)
        goto unmount;

      if (cmd[0] == '\0')
        continue;

      parse_command(cmd);
    }

  unmount:
    dump_stats();
    errno = 0;
    if (bpf_loaded_program_obj) {
      if (unmount_bpf_module(bpf_loaded_program_obj, interface) < 0) {
        print(ERROR, "Failed to detach module: %s", bpf_module_name);
      } else
        print(SUCCESS, "Module '%s' detached successfully", bpf_module_name);
    }
    atomic_store(&bpf_module_change_requested, false);
  }

  if (cleanup() != 0) {
    print(ERROR, "Cleanup failed");
    return 1;
  }
  print(SUCCESS, "exited(0)");

  return 0;
}

static void parse_command(char *input) {
  lower(input);

  char **args = strsplit(input, ' ');
  if (!args || !args[0]) {
    print(ERROR, "Invalid command");
    strsplit_free(args);
    return;
  }

  if (strcmp(args[0], STATS_COMMAND) == 0) {
    dump_stats();
  } else if (strcmp(args[0], CHANGE_MODULE_COMMAND) == 0) {
    atomic_store(&bpf_module_change_requested, true);
  } else if (strcmp(args[0], CHANGE_CONFIG_COMMAND) == 0) {
    int arg = 0;
    while (args[++arg] != NULL) {
      char **kv = strsplit(args[arg], '=');
      if (kv && kv[0] && kv[1]) {
        char key[CONFIG_KEY_SIZE] = {0};
        snprintf(key, sizeof(key), "%s", kv[0]);

        uint64_t param_value = strtoull(kv[1], NULL, 10);

        if (bpf_map_update_elem(config_map_fd, key, &param_value, BPF_ANY) != 0)
          print(ERROR, "Failed to update config map for parameter '%s'", kv[0]);
        else
          print(SUCCESS, "Parameter '%s' updated to %lu", kv[0],
                (unsigned long)param_value);
      } else
        print(ERROR, "Invalid parameter format: %s. Use name=value.",
              args[arg]);
      strsplit_free(kv);
    }
  } else if (strcmp(args[0], HELP_COMMAND) == 0)
    print(NULL,
          "Available commands:\n- %s: Dump current stats to log file\n- %s: "
          "Change module configuration\n- %s: Change loaded module\n- %s: Show "
          "this help message\n- %s: Clear screen\n- %s: Exit",
          STATS_COMMAND, CHANGE_CONFIG_COMMAND, CHANGE_MODULE_COMMAND,
          HELP_COMMAND, CLEAR_SCREEN_COMMAND, EXIT_COMMAND);
  else if (strcmp(args[0], CLEAR_SCREEN_COMMAND) == 0) {
    system("clear");
  } else if (strcmp(args[0], EXIT_COMMAND) == 0) {
    atomic_store(&active, false);
  } else if (args[0][0] != '\0')
    print(ERROR, "Unknown command, type %s for help", HELP_COMMAND);

  strsplit_free(args);
}