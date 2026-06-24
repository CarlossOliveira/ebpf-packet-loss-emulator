#include "globals.h"

#include "setup.h"
#include "cleanup.h"
#include "reporting.h"
#include "cli.h"
#include "cli/commands/help.h"
#include "cli/commands/clear.h"

#include "elf_utils.h"
#include "io_utils.h"
#include "string_utils.h"
#include "privilege_utils.h"

#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>

runtime_events_t *runtime_events;
app_context_t *ctx;

int main(const int argc, const char *argv[])
{
	if (require_root() != 0) {
		print(ERROR, "This program must be run as root");
		return 1;
	}

	if (argc == 2) {
		if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
			help_command();
			return 0;
		}
	}

	int run_flag_index = -1;
	int dump_flag_index = -1;
	if (argc > 2) {
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "--run") == 0 || strcmp(argv[i], "-r") == 0) {
				run_flag_index = i;

				i++; // Skip the next argument since it's the command to run
			} else if (strcmp(argv[i], "--dump") == 0 || strcmp(argv[i], "-d") == 0) {
				dump_flag_index = i;

				i++; // Skip the next argument since it's the dump file path
			} else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
				continue; // Help flag is handled separately at the beginning
			} else {
				print(ERROR, "Unknown argument: %s", argv[i]);
				return 1;
			}
		}
	}

	// If the dump flag is set, create the necessary directories and redirect stdout to the specified file
	if (dump_flag_index != -1) {
		char dir_path[PATH_MAX];
		snprintf(dir_path, sizeof(dir_path), "%s", argv[dump_flag_index + 1]);

		const char *dir = dirname(dir_path);

		if (strcmp(dir, ".") != 0)
			if (make_path(dir) == -1) {
				print(ERROR, "Failed to create directories for dump file");
				return 1;
			}

		// Open the dump file and redirect stdout to it
		FILE *dump_file = fopen(argv[dump_flag_index + 1], "w");
		if (!dump_file) {
			print(ERROR, "Failed to open dump file");
			return 1;
		}
		dup2(fileno(dump_file), STDOUT_FILENO);
	}

	if ((ctx = setup()) == NULL) {
		print(ERROR, "Setup failed");
		return 1;
	}

	// If the run flag is set, execute the specified command(s) and exit
	if (run_flag_index != -1) {
		char **cmd = strsplit(argv[run_flag_index + 1], ';', 0);
		if (!cmd) {
			print(ERROR, "Failed to parse command");
			cleanup(ctx);
			return 1;
		}

		for (int i = 0; cmd[i] != NULL; i++) {
			handle_input(ctx, cmd[i]);
		}

		strsplit_free(cmd);
		goto cleanup;
	}

	clear_screen_command(NULL);
	cli(ctx);

cleanup:
	if (ctx->bpf.maps.stats_map_fd > 0)
		dump_stats(ctx);

	if (cleanup(ctx) != 0 || (dump_flag_index != -1 && fclose(stdout) != 0)) {
		print(ERROR, "Cleanup failed");
		return 1;
	}

	print(SUCCESS, "Exited successfully");
	return 0;
}
