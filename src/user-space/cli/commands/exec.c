#include "exec.h"

#include "commands.h"

#include "io_utils.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

void exec_command(char **input)
{
	if (input) {
		if (HELP_FLAG) {
			print(NULL, EXEC_HELP_MESSAGE);
			return;
		}

		// Reconstruct the command string from the input arguments
		char buffer[1024];
		size_t offset = 0;
		buffer[0] = '\0';
		for (int i = 1; input[i] && offset < sizeof(buffer) - 1; i++) {
			int ret = snprintf(buffer + offset, sizeof(buffer) - offset, "%s%s", input[i],
					   input[i + 1] ? " " : "");

			if (ret < 0)
				return -1;

			if ((size_t)ret >= sizeof(buffer) - offset)
				break;

			offset += (size_t)ret;
		}

		int ret = system(buffer);
		if (ret == -1) {
			print(ERROR, "Failed to execute command: %s", strerror(errno));
		} else if (WIFEXITED(ret) && WEXITSTATUS(ret) != 0) {
			print(ERROR, "Command exited with status %d", WEXITSTATUS(ret));
		} else if (WIFSIGNALED(ret)) {
			print(ERROR, "Command terminated by signal %d", WTERMSIG(ret));
		}
	}
}
