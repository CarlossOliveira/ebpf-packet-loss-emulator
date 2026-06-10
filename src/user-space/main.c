#include "globals.h"

#include "setup.h"
#include "cleanup.h"
#include "cli.h"
#include "cli/commands/help.h"
#include "cli/commands/clear.h"

#include "elf_utils.h"
#include "io_utils.h"
#include "privilege_utils.h"
#include "string_utils.h"

#include <stdatomic.h>

runtime_events_t *runtime_events;
app_context_t *ctx;

int main(int argc, char *argv[])
{
	if (require_root() != 0) {
		print(ERROR, "This program must be run as root");
		return 1;
	}

	if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
		help_command();
		return 0;
	}

	if ((ctx = setup()) == NULL) {
		print(ERROR, "Setup failed");
		return 1;
	}

	clear_screen_command(NULL);
	cli(ctx);

	if (cleanup(ctx) != 0) {
		print(ERROR, "Cleanup failed");
		return 1;
	}
	print(SUCCESS, "Exited successfully");

	return 0;
}
