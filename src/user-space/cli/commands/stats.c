#include "stats.h"

#include "globals.h"

#include "reporting.h"

#include "commands.h"

#include "io_utils.h"
#include "time_utils.h"

#include <string.h>

void stats_command(const app_context_t *ctx, char **input)
{
	if (input)
		if (HELP_FLAG) {
			print(NULL, STATS_HELP_MESSAGE);
			return;
		}

	if (strlen(ctx->bpf.module_name) == 0) {
		print(WARNING, "No module loaded");
		return;
	}

	dump_stats(ctx);
}
