#include "unload.h"

#include "globals.h"

#include "loader.h"

#include "commands.h"
#include "io_utils.h"
#include "string_utils.h"

void unload_module_command(app_context_t *ctx, char **input)
{
	if (input)
		if (HELP_FLAG) {
			print(NULL, UNLOAD_HELP_MESSAGE);
			return;
		}

	if (strlen(ctx->bpf.module_name) == 0) {
		print(WARNING, "No module loaded");
		return;
	}

	dump_stats(ctx);

	if (unload_bpf_module(ctx) != 0) {
		print(ERROR, "Failed to unload module: %s", ctx->bpf.module_name);
	} else
		print(SUCCESS, "Module '%s' unloaded successfully", ctx->bpf.module_name);

	snprintf(ctx->bpf.module_name, sizeof(ctx->bpf.module_name), "%s", "");
	ctx->bpf.module_name[sizeof(ctx->bpf.module_name) - 1] = '\0';
}
