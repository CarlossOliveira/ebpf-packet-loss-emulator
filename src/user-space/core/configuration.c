#include "configuration.h"

#include "globals.h"

#include "bpf_manager.h"

#include "io_utils.h"
#include "elf_utils.h"
#include "string_utils.h"

#include <linux/if_link.h>
#include <net/if.h>
#include <stdlib.h>

void set_attachment_points(app_context_t *ctx, const char *raw_attach_points)
{
	char **attach_points = strsplit(raw_attach_points, '|', 0);
	if (!attach_points) {
		print(ERROR, "Failed to split attachment point");
		return;
	}

	ctx->interface.tc_attach_points = 0;
	ctx->interface.xdp_attach_point = 0;
	short xdp_flags = 0;
	for (int i = 0; attach_points[i]; i++) {
		remove_char(attach_points[i], ' '); // Remove any spaces

		if (strcmp(attach_points[i], "ingress") == 0 || strcmp(attach_points[i], "BPF_TC_INGRESS") == 0)
			ctx->interface.tc_attach_points |= BPF_TC_INGRESS;
		else if (strcmp(attach_points[i], "egress") == 0 || strcmp(attach_points[i], "BPF_TC_EGRESS") == 0)
			ctx->interface.tc_attach_points |= BPF_TC_EGRESS;
		else if (strcmp(attach_points[i], "offload") == 0 ||
			 strcmp(attach_points[i], "XDP_FLAGS_HW_MODE") == 0) {
			ctx->interface.xdp_attach_point |= XDP_FLAGS_HW_MODE;
			xdp_flags++;
		} else if (strcmp(attach_points[i], "driver") == 0 ||
			   strcmp(attach_points[i], "XDP_FLAGS_DRV_MODE") == 0) {
			ctx->interface.xdp_attach_point |= XDP_FLAGS_DRV_MODE;
			xdp_flags++;
		} else if (strcmp(attach_points[i], "generic") == 0 ||
			   strcmp(attach_points[i], "XDP_FLAGS_SKB_MODE") == 0) {
			ctx->interface.xdp_attach_point |= XDP_FLAGS_SKB_MODE;
			xdp_flags++;
		} else {
			print(WARNING, "Invalid attachment point: %s. Skipping...", attach_points[i]);
		}

		if (xdp_flags > 1) {
			print(ERROR, "Multiple XDP attachment modes specified. Please "
				     "specify only one of: generic, driver, offload.");
			strsplit_free(attach_points);
			return;
		}
	}

	strsplit_free(attach_points);
}

void set_module(app_context_t *ctx, char *module_name)
{
	if (!module_name) {
		print(ERROR, "Module name is required. Usage: load --module \"<module_name>\"");
		return;
	}

	// Load config keys from the eBPF object section
	snprintf(ctx->bpf.module_name, sizeof(ctx->bpf.module_name), "%s", module_name);
	ctx->bpf.module_name[sizeof(ctx->bpf.module_name) - 1] = '\0';

	ctx->bpf.object = load_bpf_module(ctx);
	if (ctx->bpf.object == NULL) {
		print(ERROR, "Failed to attach module: %s", module_name);
	} else {
		print(SUCCESS, "Module '%s' loaded successfully", ctx->bpf.module_name);

		char bpf_elf_filename[512];
		snprintf(bpf_elf_filename, sizeof(bpf_elf_filename), "%s/%s.bpf.o", BPF_OBJECT_DIR,
			 ctx->bpf.module_name);
		size_t size;
		char *bpf_params = read_elf_section(bpf_elf_filename, ".config_keys", &size);

		if (!bpf_params) {
			print(WARNING, "No config keys found in the eBPF object. Make sure to define a .config_keys "
				       "section in your eBPF C code if you want to use dynamic configuration.");
		} else {
			for (size_t i = 0; i < (size / CONFIG_KEY_SIZE); i++) {
				snprintf(ctx->bpf.config_keys[i], CONFIG_KEY_SIZE, "%s",
					 bpf_params + (i * CONFIG_KEY_SIZE));
				if (*(bpf_params + (i * CONFIG_KEY_SIZE)) == '\0') // Sentinel indicating no more valid
					// keys
					break;
			}
			free(bpf_params);
		}
	}
}
