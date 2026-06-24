#include "config.h"

#include "globals.h"

#include "commands.h"
#include "io_utils.h"
#include "string_utils.h"

#include <bpf/bpf.h>
#include <stdlib.h>
#include <string.h>

void config_module_command(app_context_t *ctx, char **input)
{
	if (input) {
		if (HELP_FLAG) {
			print(NULL, CHANGE_CONFIG_HELP_MESSAGE);
			return;
		}

		if (strlen(ctx->bpf.module_name) == 0) {
			print(WARNING, "No module loaded");
			return;
		}

		for (int arg = 1; input[arg]; arg++) {
			char **kv = strsplit(input[arg], '=', 2);

			if (!kv || !kv[0] || !kv[1]) {
				print(ERROR, "Invalid parameter format: Expected key=value, got %s", input[arg]);
				strsplit_free(kv);
				continue;
			}

			if (strcmp(ctx->bpf.config_keys[0], kv[0]) != 0) {
				print(ERROR, "Unknown config key: %s", kv[0]);
				print(NULL, "        Valid config options:");
				for (int i = 0; i < MAX_CONFIG_ENTRIES && strlen(ctx->bpf.config_keys[i]) > 0; i++) {
					print(NULL, "          - %s", ctx->bpf.config_keys[i]);
				}
				strsplit_free(kv);
				continue;
			}

			uint64_t value = strtoull(kv[1], NULL, 10);

			if (bpf_map_update_elem(ctx->bpf.maps.config_map_fd, kv[0], &value, BPF_ANY) != 0) {
				print(ERROR, "Failed to update config param '%s'", kv[0]);
			} else {
				print(SUCCESS, "Parameter '%s' updated to %lu", kv[0], (unsigned long)value);
			}

			strsplit_free(kv);
		}
	}
}