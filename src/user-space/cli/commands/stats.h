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

/**
 * @brief Handles the 'stats' command, displaying statistics for the currently loaded eBPF module.
 *
 * @param ctx A pointer to the application context containing the BPF and interface information.
 * @param input An array of strings representing the command-line arguments.
 */
void stats_command(const app_context_t *ctx, char **input);

#endif //! STATS_H
