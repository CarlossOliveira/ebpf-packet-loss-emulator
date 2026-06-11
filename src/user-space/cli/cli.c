#include "cli.h"

#include "globals.h"

#include "io_utils.h"
#include "string_utils.h"

#include "loader.h"

#include "commands.h"

#include <readline/history.h>
#include <readline/readline.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void handle_input(app_context_t *ctx, const char *input);

void cli(app_context_t *ctx)
{
	while (!atomic_load(&runtime_events->shutdown_requested)) {
		char cmd[256] = {0};
		if (input((char *)"ebpf-packet-loss-emulator> ", cmd, sizeof(cmd)))
			return;

		if (atomic_load(&runtime_events->module_switch_requested)) {
			atomic_store(&runtime_events->module_switch_requested, false);
			unload_bpf_module(ctx);
			clear_screen_command(NULL);
			list_modules_command(NULL);

			char module_name[256];
			input((char *)"Select one of the available modules: ", module_name, sizeof(module_name));

			if (strlen(module_name) == 0) {
				print(WARNING, "No module name entered. Please try again.");
				continue;
			}

			char interface_name[256];
			input((char *)"Enter the interface name to attach to (e.g., eth0): ", interface_name,
			      sizeof(interface_name));

			if (strlen(interface_name) == 0) {
				print(WARNING, "No interface name entered. Please try again.");
				continue;
			}

			load_module_command(
			    ctx, (char *[]){"load", "--module", module_name, "--interface", interface_name, NULL});
			continue;
		}

		if (strlen(cmd) > 0)
			handle_input(ctx, cmd);
	}
}

int input(char *prompt, char *buffer, size_t size)
{
	if (!buffer || size == 0)
		return 1;

	buffer[0] = '\0';

	fflush(stdout); // Ensure prompt is printed before input
	char *line = readline(prompt ? prompt : "");

	if (!line) {
		atomic_store(&runtime_events->shutdown_requested, true);
		return 1;
	}

	if (line[0] != '\0') {
		const HIST_ENTRY *last = history_get(history_length);

		if (!last || strcmp(last->line, line) != 0) {
			add_history(line);
		}
	}

	snprintf(buffer, size, "%s", line);
	free(line);
	return 0;
}

static void handle_input(app_context_t *ctx, const char *input)
{
	char **args = strsplit(input, ' ', 0);
	if (!args || !args[0]) {
		strsplit_free(args);
		return;
	}

	if (strcmp(args[0], HELP_COMMAND) == 0)
		help_command();
	else if (strcmp(args[0], LIST_MODULES_COMMAND) == 0)
		list_modules_command(args);
	else if (strcmp(args[0], LOAD_MODULE_COMMAND) == 0)
		load_module_command(ctx, args);
	else if (strcmp(args[0], UNLOAD_MODULE_COMMAND) == 0)
		unload_module_command(ctx, args);
	else if (strcmp(args[0], CONFIG_MODULE_COMMAND) == 0)
		config_module_command(ctx, args);
	else if (strcmp(args[0], STATS_COMMAND) == 0)
		stats_command(ctx, args);
	else if (strcmp(args[0], SET_DEFAULT_COMMAND) == 0)
		set_default_command(ctx, args);
	else if (strcmp(args[0], CLEAR_SCREEN_COMMAND) == 0)
		clear_screen_command(args);
	else if (strcmp(args[0], EXIT_COMMAND) == 0)
		exit_command(args);
	else
		print(ERROR, "Unknown command: %s", args[0]);

	strsplit_free(args);
}
