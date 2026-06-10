#ifndef EXIT_H
#define EXIT_H

#include "globals.h"

#define EXIT_COMMAND "exit"

#define EXIT_HELP_MESSAGE                                                                                              \
	"NAME\n"                                                                                                       \
	"    exit - Terminate the application\n"                                                                       \
	"\n"                                                                                                           \
	"SYNOPSIS\n"                                                                                                   \
	"    exit [OPTIONS]\n"                                                                                         \
	"\n"                                                                                                           \
	"DESCRIPTION\n"                                                                                                \
	"    Terminate the CLI session and exit the application.\n"                                                    \
	"    Any loaded eBPF modules are detached and all allocated\n"                                                 \
	"    resources are released before shutdown.\n"                                                                \
	"\n"                                                                                                           \
	"OPTIONS\n"                                                                                                    \
	"    -h, --help\n"                                                                                             \
	"        Display this help message and exit.\n"                                                                \
	"\n"                                                                                                           \
	"EXAMPLES\n"                                                                                                   \
	"    exit\n"                                                                                                   \
	"    exit --help\n"

void exit_command(char **input);

#endif // !EXIT_H
