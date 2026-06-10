#include "config.h"

#include "globals.h"

#include "commands.h"
#include "io_utils.h"
#include "string_utils.h"

#include <bpf/bpf.h>
#include <stdlib.h>

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

		const char **raw_config_parameters = (const char **)(input + 1);
		for (int arg = 1; raw_config_parameters[arg]; arg++) {
			char **kv = strsplit(raw_config_parameters[arg], '=', 2);
			if (!kv || !kv[0] || !kv[1]) {
				print(ERROR, "Invalid parameter format: %s. Expected format is key=value. Skipping...",
				      raw_config_parameters[arg]);
				continue;
			}

			char *key = kv[0];
			uint64_t value = strtoull(kv[1], NULL, 10);

			if (bpf_map_update_elem(ctx->bpf.maps.config_map_fd, key, &value, BPF_ANY) != 0)
				print(ERROR,
				      "Failed to update config map for "
				      "parameter '%s'",
				      kv[0]);
			else
				print(SUCCESS, "Parameter '%s' updated to %lu", kv[0], (unsigned long)value);
			strsplit_free(kv);
		}
		print(SUCCESS, "Configuration updated successfully");
	}
}
