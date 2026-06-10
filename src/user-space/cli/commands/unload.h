#ifndef UNLOAD_H
#define UNLOAD_H

#include "globals.h"

#define UNLOAD_MODULE_COMMAND "unload"

#define UNLOAD_HELP_MESSAGE                                                                                            \
	"NAME\n"                                                                                                       \
	"    unload - Detach and unload the active eBPF module\n"                                                      \
	"\n"                                                                                                           \
	"SYNOPSIS\n"                                                                                                   \
	"    unload [OPTIONS]\n"                                                                                       \
	"\n"                                                                                                           \
	"DESCRIPTION\n"                                                                                                \
	"    Detach the currently loaded eBPF program from its network\n"                                              \
	"    interface attachment point(s) and release all associated\n"                                               \
	"    resources.\n"                                                                                             \
	"\n"                                                                                                           \
	"    If no module is currently loaded, the command has no effect.\n"                                           \
	"\n"                                                                                                           \
	"OPTIONS\n"                                                                                                    \
	"    -h, --help\n"                                                                                             \
	"        Display this help message and exit.\n"                                                                \
	"\n"                                                                                                           \
	"EXAMPLES\n"                                                                                                   \
	"    unload\n"                                                                                                 \
	"    unload --help\n"

void unload_module_command(app_context_t *ctx, char **input);

#endif // !UNLOAD_H
