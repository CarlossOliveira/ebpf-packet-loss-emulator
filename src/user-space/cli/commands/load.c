#include "load.h"

#include "globals.h"

#include "loader.h"

#include "commands.h"
#include "io_utils.h"
#include "elf_utils.h"
#include "string_utils.h"

#include <stdlib.h>
#include <linux/if_link.h>
#include <net/if.h>

static void select_module(app_context_t *ctx, char *module_name);

void load_module_command(app_context_t *ctx, char **input)
{
	if (input) {
		if (HELP_FLAG) {
			print(NULL, LOAD_HELP_MESSAGE);
			return;
		}

		char attachment_points[40] = {0};
		char interface[IFNAMSIZ] = {0};
		char module_name[256] = {0};

		int arg = 1; // Start from 1 to skip the "load" command itself
		int flags_chosen = 0;
		while (input[arg]) {
			char *current_arg = input[arg];

			if (strcmp(current_arg, "--attach-point") == 0 || strcmp(current_arg, "-a") == 0) {
				if (!input[arg + 1]) {
					print(ERROR, "Missing attach point value");
					return;
				}

				snprintf(attachment_points, sizeof(attachment_points), "%s", input[arg + 1]);
				attachment_points[sizeof(attachment_points) - 1] = '\0';

				flags_chosen++;
				arg++; // Skip the interface name argument since we've already processed it
			} else if (strcmp(current_arg, "--module") == 0 || strcmp(current_arg, "-m") == 0) {
				if (!input[arg + 1]) {
					print(ERROR, "Missing module name");
					return;
				}

				snprintf(module_name, sizeof(module_name), "%s", input[arg + 1]);
				module_name[sizeof(module_name) - 1] = '\0';

				flags_chosen++;
				arg++; // Skip the interface name argument since we've already processed it
			} else if (strcmp(current_arg, "--interface") == 0 || strcmp(current_arg, "-i") == 0) {
				if (!input[arg + 1]) {
					print(ERROR, "Missing interface name");
					return;
				}

				snprintf(interface, sizeof(interface), "%s", input[arg + 1]);
				interface[sizeof(interface) - 1] = '\0';

				flags_chosen++;
				arg++; // Skip the interface name argument since we've already processed it
			} else {
				print(ERROR, "Unknown argument: %s.", input[arg]);
				return;
			}
			arg++;
		}

		// Validate required parameters - either from input or defaults
		if ((strlen(module_name) == 0 && strlen(ctx->bpf.module_name) == 0) ||
		    (strlen(interface) == 0 && strlen(ctx->interface.default_interface_name) == 0) ||
		    (strlen(attachment_points) == 0 && ctx->interface.default_tc_attach_points == 0 &&
		     ctx->interface.default_xdp_attach_point == 0) || flags_chosen == 0 || flags_chosen > 3) {
			print(ERROR, "Module name and interface are required. Usage: load --module \"<module_name>\" "
				     "--interface \"<interface_name>\"");
			return;
		}

		// Load module and set up context
		if (strlen(attachment_points) > 0)
			select_attachment_points(ctx, attachment_points);
		else {
			ctx->interface.tc_attach_points = ctx->interface.default_tc_attach_points;
			ctx->interface.xdp_attach_point = ctx->interface.default_xdp_attach_point;
		}

		if (strlen(interface) > 0) {
			snprintf(ctx->interface.name, sizeof(ctx->interface.name), "%s", interface);
			ctx->interface.name[sizeof(ctx->interface.name) - 1] = '\0';
		} else {
			snprintf(ctx->interface.name, sizeof(ctx->interface.name), "%s",
				 ctx->interface.default_interface_name);
			ctx->interface.name[sizeof(ctx->interface.name) - 1] = '\0';
		}

		select_module(ctx, module_name);
	}
}

void select_attachment_points(app_context_t *ctx, const char *raw_attach_points)
{
	char **attach_points = strsplit(raw_attach_points, '|', 0);
	if (!attach_points) {
		print(ERROR, "Failed to split attach point");
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
			print(WARNING, "Invalid attach point: %s. Skipping...", attach_points[i]);
		}

		if (xdp_flags > 1) {
			print(ERROR, "Multiple XDP attach modes specified. Please "
				     "specify only one of: generic, driver, offload.");
			strsplit_free(attach_points);
			return;
		}
	}

	strsplit_free(attach_points);
}

static void select_module(app_context_t *ctx, char *module_name)
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
		print(SUCCESS, "Module '%s' attached successfully", ctx->bpf.module_name);

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
