#ifndef SET_DEFAULT_H
#define SET_DEFAULT_H

#include "globals.h"

#define SET_DEFAULT_COMMAND "set-default"

#define SET_DEFAULT_HELP_MESSAGE                                                                                       \
	"NAME\n"                                                                                                       \
	"    set-default - Configure default values for module loading\n"                                              \
	"\n"                                                                                                           \
	"SYNOPSIS\n"                                                                                                   \
	"    set-default interface [<interface_name>]\n"                                                               \
	"    set-default attachment-point [<attach_point>]\n"                                                          \
	"    set-default clear\n"                                                                                      \
	"    set-default -h\n"                                                                                         \
	"    set-default --help\n"                                                                                     \
	"\n"                                                                                                           \
	"DESCRIPTION\n"                                                                                                \
	"    Configure default values used by the load command.\n"                                                     \
	"\n"                                                                                                           \
	"    When a parameter is omitted from the load command,\n"                                                     \
	"    the corresponding configured default value is used.\n"                                                    \
	"\n"                                                                                                           \
	"SUBCOMMANDS\n"                                                                                                \
	"    interface [<interface_name>]\n"                                                                           \
	"        Set the default network interface.\n"                                                                 \
	"\n"                                                                                                           \
	"        If no interface name is specified, the current\n"                                                     \
	"        default interface is cleared.\n"                                                                      \
	"\n"                                                                                                           \
	"    attachment-point [<attach_point>]\n"                                                                      \
	"        Set the default attachment point(s).\n"                                                               \
	"\n"                                                                                                           \
	"        Multiple attachment points may be specified\n"                                                        \
	"        using '|'.\n"                                                                                         \
	"\n"                                                                                                           \
	"        Any combination of TC attachment points is\n"                                                         \
	"        allowed:\n"                                                                                           \
	"            ingress, BPF_TC_INGRESS\n"                                                                        \
	"            egress,  BPF_TC_EGRESS\n"                                                                         \
	"\n"                                                                                                           \
	"        At most one XDP attachment mode may be\n"                                                             \
	"        specified. The following modes are mutually\n"                                                        \
	"        exclusive:\n"                                                                                         \
	"            offload, XDP_FLAGS_HW_MODE\n"                                                                     \
	"            driver,  XDP_FLAGS_DRV_MODE\n"                                                                    \
	"            generic, XDP_FLAGS_SKB_MODE\n"                                                                    \
	"\n"                                                                                                           \
	"        If no attachment point is specified, the\n"                                                           \
	"        default attachment points are restored to\n"                                                          \
	"        their default values:\n"                                                                              \
	"            TC:  ingress (BPF_TC_INGRESS)\n"                                                                  \
	"            XDP: generic (XDP_FLAGS_SKB_MODE)\n"                                                              \
	"\n"                                                                                                           \
	"    clear\n"                                                                                                  \
	"        Remove all configured default values.\n"                                                              \
	"\n"                                                                                                           \
	"OPTIONS\n"                                                                                                    \
	"    -h, --help\n"                                                                                             \
	"        Display this help message and exit.\n"                                                                \
	"\n"                                                                                                           \
	"EXAMPLES\n"                                                                                                   \
	"    set-default interface eth0\n"                                                                             \
	"    set-default attachment-point ingress\n"                                                                   \
	"    set-default attachment-point ingress|egress\n"                                                            \
	"    set-default interface\n"                                                                                  \
	"    set-default clear\n"                                                                                      \
	"    set-default --help\n"

void set_default_command(app_context_t *ctx, char **input);

#endif // !SET_DEFAULT_H
