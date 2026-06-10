#ifndef LIST_H
#define LIST_H

#include "globals.h"

#define LIST_MODULES_COMMAND "list"

#define LIST_HELP_MESSAGE                                                                                              \
	"NAME\n"                                                                                                       \
	"    list - List available eBPF modules\n"                                                                     \
	"\n"                                                                                                           \
	"SYNOPSIS\n"                                                                                                   \
	"    list [OPTIONS]\n"                                                                                         \
	"\n"                                                                                                           \
	"DESCRIPTION\n"                                                                                                \
	"    Display the names of eBPF modules available in the module\n"                                              \
	"    search directory.\n"                                                                                      \
	"\n"                                                                                                           \
	"OPTIONS\n"                                                                                                    \
	"    -h, --help\n"                                                                                             \
	"        Display this help message and exit.\n"                                                                \
	"\n"                                                                                                           \
	"EXAMPLES\n"                                                                                                   \
	"    list\n"                                                                                                   \
	"    list --help\n"

void list_modules_command(char **input);

#endif // !LIST_H
