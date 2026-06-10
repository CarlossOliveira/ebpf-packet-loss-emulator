#include "clear.h"

#include "globals.h"

#include "commands.h"
#include "io_utils.h"

#include <stdlib.h>

void clear_screen_command(char **input)
{
	if (input)
		if (HELP_FLAG) {
			print(NULL, CLEAR_HELP_MESSAGE);
			return;
		}

	system("clear");
	print(NULL, "Press Ctrl+C to exit, Ctrl+\\ to change/load module or help for more information.");
}
