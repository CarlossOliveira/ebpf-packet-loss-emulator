#include "set-default.h"

#include "globals.h"

#include "configuration.h"

#include "commands.h"

#include "io_utils.h"

#include <linux/if_link.h>

void set_default_command(app_context_t *ctx, char **input)
{
	if (input) {
		if (HELP_FLAG) {
			print(NULL, SET_DEFAULT_HELP_MESSAGE);
			return;
		}

		if (!input[1]) {
			print(ERROR,
			      "Missing default type. Valid types are 'interface', 'attachment-point', and 'clear'.");
			return;
		}

		if (strcmp(input[1], "interface") == 0) {
			if (!input[2]) {
				ctx->interface.default_interface_name[0] =
				    '\0'; // Clear default interface if no name provided
				print(NULL, "Default interface cleared.");
			} else {
				snprintf(ctx->interface.default_interface_name,
					 sizeof(ctx->interface.default_interface_name), "%s", input[2]);
				ctx->interface.default_interface_name[sizeof(ctx->interface.default_interface_name) -
								      1] = '\0';
			}
		} else if (strcmp(input[1], "attachment-point") == 0) {
			if (!input[2]) {
				ctx->interface.default_tc_attach_points = BPF_TC_INGRESS;
				ctx->interface.default_xdp_attach_point = XDP_FLAGS_SKB_MODE;
				print(NULL, "Default attachment points cleared.");
			} else {
				set_attachment_points(ctx, input[2]);
			}
		} else if (strcmp(input[1], "clear") == 0) {
			ctx->interface.default_interface_name[0] = '\0';
			ctx->interface.default_tc_attach_points = BPF_TC_INGRESS;
			ctx->interface.default_xdp_attach_point = XDP_FLAGS_SKB_MODE;
			print(NULL, "All default values cleared.");
		} else {
			print(ERROR,
			      "Unknown default type: %s. Valid types are 'interface', 'attachment-point', and 'clear'.",
			      input[1]);
		}
	}
}
