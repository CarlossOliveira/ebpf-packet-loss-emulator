#include "cleanup.h"

#include "globals.h"

#include "bpf_manager.h"

#include "elf_utils.h"
#include "io_utils.h"

#include <stdlib.h>
#include <net/if.h>
#include <unistd.h>

int cleanup(app_context_t *ctx)
{
	int status = 0;

	if (ctx->bpf.object)
		status = unload_bpf_module(ctx);

	if (ctx->bpf.maps.stats_map_fd >= 0) {
		close(ctx->bpf.maps.stats_map_fd);
		ctx->bpf.maps.stats_map_fd = -1;
	}

	free(ctx);

	return status;
}
