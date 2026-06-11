#include "exit.h"

#include "globals.h"

#include "commands.h"

#include "io_utils.h"

void exit_command(char **input)
{
	if (input)
		if (HELP_FLAG) {
			print(NULL, EXIT_HELP_MESSAGE);
			return;
		}

	atomic_store(&runtime_events->shutdown_requested, true);
}
