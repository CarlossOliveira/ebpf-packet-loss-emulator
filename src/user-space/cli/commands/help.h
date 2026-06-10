#ifndef HELP_H
#define HELP_H

#define HELP_COMMAND "help"

#include "commands.h"

// Help message definition for easy maintenance and readability
#define HELP_MESSAGE                                                                                                   \
	BOLD "NAME\n" RESET "    ebpf-packet-loss-emulator - Interactive eBPF packet loss emulation framework\n"       \
	     "\n" BOLD "SYNOPSIS\n" RESET "    ebpf-packet-loss-emulator [OPTIONS]\n"                                  \
	     "\n" BOLD "DESCRIPTION\n" RESET "    Provides an interactive command-line interface for loading,\n"       \
	     "    configuring, monitoring and unloading eBPF packet loss\n"                                            \
	     "    emulation modules.\n"                                                                                \
	     "\n"                                                                                                      \
	     "    Documentation, source code and issue reporting:\n"                                                   \
	     "    https://github.com/CarlossOliveira/ebpf-packet-loss-emulator\n"                                      \
	     "\n" BOLD "OPTIONS\n" RESET "    -h, --help\n"                                                            \
	     "        Display this help message and exit.\n"                                                           \
	     "\n" BOLD "AVAILABLE COMMANDS\n" RESET "    list      List available eBPF modules.\n"                     \
	     "    load          Load and attach an eBPF module.\n"                                                     \
	     "    unload        Detach and unload the active module.\n"                                                \
	     "    config        Modify module configuration parameters.\n"                                             \
	     "    stats         Display runtime statistics.\n"                                                         \
	     "    set-default   Configure default values used by the load command.\n"                                  \
	     "    clear         Clear the terminal screen.\n"                                                          \
	     "    help          Display this help message.\n"                                                          \
	     "    exit          Exit the application.\n"                                                               \
	     "\n" BOLD "COMMAND DOCUMENTATION\n" RESET "    Each command supports the -h and --help options to\n"      \
	     "    display command-specific documentation, available\n"                                                 \
	     "    options and usage examples.\n"                                                                       \
	     "\n"                                                                                                      \
	     "    Examples:\n"                                                                                         \
	     "        exit -h\n"                                                                                       \
	     "        stats --help\n"                                                                                  \
	     "\n" BOLD "GETTING STARTED\n" RESET "    list\n"                                                          \
	     "    load -m tc_bernoulli -i eth0\n"                                                                      \
	     "    config loss_rate=10\n"                                                                               \
	     "    stats\n"                                                                                             \
	     "\n" BOLD "SEE ALSO\n" RESET "    bpftool(8), tc(8), ip-link(8)\n"

void help_command(void);

#endif // !HELP_H
