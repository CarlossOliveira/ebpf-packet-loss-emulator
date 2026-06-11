#ifndef LOAD_H
#define LOAD_H

#include "globals.h"

#define LOAD_MODULE_COMMAND "load"

#define LOAD_HELP_MESSAGE                                                                                              \
	"NAME\n"                                                                                                       \
	"    load - Load and attach an eBPF module\n"                                                                  \
	"\n"                                                                                                           \
	"SYNOPSIS\n"                                                                                                   \
	"    load -m <module_name> -i <interface_name> [-a <attach_point>]\n"                                          \
	"    load [--help|-h]\n"                                                                                       \
	"\n"                                                                                                           \
	"DESCRIPTION\n"                                                                                                \
	"    Load an eBPF module and attach it to a network\n"                                                         \
	"    interface.\n"                                                                                             \
	"\n"                                                                                                           \
	"    The module name must be specified explicitly.\n"                                                          \
	"    The target interface may be specified explicitly\n"                                                       \
	"    or obtained from a previously configured default\n"                                                       \
	"    value."                                                                                                   \
	"\n"                                                                                                           \
	"    If no attachment point is specified, the configured\n"                                                    \
	"    default attachment point is used. If no default is\n"                                                     \
	"    configured, the following values are applied:\n"                                                          \
	"\n"                                                                                                           \
	"        TC:  ingress (BPF_TC_INGRESS)\n"                                                                      \
	"        XDP: generic (XDP_FLAGS_SKB_MODE)\n"                                                                  \
	"\n"                                                                                                           \
	"OPTIONS\n"                                                                                                    \
	"    -a, --attachment-point <attachment_points>\n"                                                             \
	"        Attachment point(s) for the eBPF program.\n"                                                          \
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
	"    -m, --module <module_name>\n"                                                                             \
	"        Name of the eBPF module to load.\n"                                                                   \
	"\n"                                                                                                           \
	"    -i, --interface <interface_name>\n"                                                                       \
	"        Network interface to attach the eBPF program\n"                                                       \
	"        to.\n"                                                                                                \
	"\n"                                                                                                           \
	"    -h, --help\n"                                                                                             \
	"        Display this help message and exit.\n"                                                                \
	"\n"                                                                                                           \
	"EXAMPLES\n"                                                                                                   \
	"    load -m tc_bernoulli -i eth0\n"                                                                           \
	"    load -a ingress -m tc_bernoulli -i veth0\n"                                                               \
	"    load -a ingress|egress -m tc_bernoulli -i eth0\n"                                                         \
	"    load -a ingress|egress|driver -m tc_bernoulli -i veth0\n"                                                 \
	"    load -a offload -m xdp_bernoulli -i eth0\n"                                                               \
	"    load --help\n"

/**
 * @brief Handles the 'load' command, loading and attaching an eBPF module based on user input.
 *
 * @param ctx A pointer to the application context containing the BPF and interface information.
 * @param input An array of strings representing the command-line arguments.
 */
void load_module_command(app_context_t *ctx, char **input);

#endif // !LOAD_H
