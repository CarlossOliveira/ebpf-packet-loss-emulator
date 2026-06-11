#include "load.h"

#include "globals.h"

#include "bpf_manager.h"
#include "configuration.h"

#include "commands.h"

#include "io_utils.h"
#include "elf_utils.h"
#include "string_utils.h"

#include <stdlib.h>

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

			if (strcmp(current_arg, "--attachment-point") == 0 || strcmp(current_arg, "-a") == 0) {
				if (!input[arg + 1]) {
					print(ERROR, "Missing attachment point value");
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
		     ctx->interface.default_xdp_attach_point == 0) ||
		    flags_chosen == 0 || flags_chosen > 3) {
			print(ERROR, "Module name and interface are required. Usage: load --module \"<module_name>\" "
				     "--interface \"<interface_name>\"");
			return;
		}

		// Load module and set up context
		if (strlen(attachment_points) > 0)
			set_attachment_points(ctx, attachment_points);
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

		set_module(ctx, module_name);
	}
}
