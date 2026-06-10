#ifndef CONFIG_H
#define CONFIG_H

#include "globals.h"

#define CONFIG_MODULE_COMMAND "config"

#define CHANGE_CONFIG_HELP_MESSAGE                                                                                     \
	"NAME\n"                                                                                                       \
	"    config - Update eBPF module configuration parameters\n"                                                   \
	"\n"                                                                                                           \
	"SYNOPSIS\n"                                                                                                   \
	"    config <key>=<value> [<key>=<value> ...]\n"                                                               \
	"\n"                                                                                                           \
	"    config [OPTIONS]\n"                                                                                       \
	"\n"                                                                                                           \
	"DESCRIPTION\n"                                                                                                \
	"    Updates one or more configuration parameters of the currently\n"                                          \
	"    loaded eBPF module.\n"                                                                                    \
	"\n"                                                                                                           \
	"OPTIONS\n"                                                                                                    \
	"    -h, --help\n"                                                                                             \
	"        Display this help message and exit.\n"                                                                \
	"\n"                                                                                                           \
	"PARAMETERS\n"                                                                                                 \
	"    <key>\n"                                                                                                  \
	"        Configuration parameter name.\n"                                                                      \
	"\n"                                                                                                           \
	"    <value>\n"                                                                                                \
	"        New value to assign to the parameter.\n"                                                              \
	"\n"                                                                                                           \
	"EXAMPLES\n"                                                                                                   \
	"    config packet_loss_percentage=100\n"                                                                      \
	"    config loss_rate=10 delay_ms=100\n"                                                                       \
	"    config --help\n"

void config_module_command(app_context_t *ctx, char **input);

#endif // !CONFIG_H
