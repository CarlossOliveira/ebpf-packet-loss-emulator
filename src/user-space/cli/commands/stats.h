#ifndef STATS_H
#define STATS_H

#include "globals.h"

#define STATS_COMMAND "stats"

#define STATS_HELP_MESSAGE                                                                                             \
	"NAME\n"                                                                                                       \
	"    stats - Display module statistics\n"                                                                      \
	"\n"                                                                                                           \
	"SYNOPSIS\n"                                                                                                   \
	"    stats [OPTIONS]\n"                                                                                        \
	"\n"                                                                                                           \
	"DESCRIPTION\n"                                                                                                \
	"    Display statistics collected by the currently loaded\n"                                                   \
	"    eBPF module.\n"                                                                                           \
	"\n"                                                                                                           \
	"    A module must be loaded before this command can be\n"                                                     \
	"    used.\n"                                                                                                  \
	"\n"                                                                                                           \
	"OPTIONS\n"                                                                                                    \
	"    -h, --help\n"                                                                                             \
	"        Display this help message and exit.\n"                                                                \
	"\n"                                                                                                           \
	"EXAMPLES\n"                                                                                                   \
	"    stats\n"                                                                                                  \
	"    stats --help\n"

void stats_command(const app_context_t *ctx, char **input);
int dump_stats(const app_context_t *ctx);

#endif //! STATS_H
