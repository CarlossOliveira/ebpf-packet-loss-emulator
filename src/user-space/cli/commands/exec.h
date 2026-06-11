#ifndef EXEC_H
#define EXEC_H

#define EXEC_COMMAND "exec"

#define EXEC_HELP_MESSAGE                                                                                              \
	"NAME\n"                                                                                                       \
	"    exec - Execute a shell command\n"                                                                         \
	"\n"                                                                                                           \
	"SYNOPSIS\n"                                                                                                   \
	"    exec <command> [arguments...]\n"                                                                          \
	"    exec [OPTIONS]\n"                                                                                         \
	"\n"                                                                                                           \
	"DESCRIPTION\n"                                                                                                \
	"    Execute a shell command from within the CLI.\n"                                                           \
	"\n"                                                                                                           \
	"    The specified command is executed using the\n"                                                            \
	"    host system shell. Any additional arguments\n"                                                            \
	"    are passed directly to the command.\n"                                                                    \
	"\n"                                                                                                           \
	"    This command can be used to execute external\n"                                                           \
	"    programs, inspect system state, or perform\n"                                                             \
	"    administrative tasks without leaving the CLI.\n"                                                          \
	"\n"                                                                                                           \
	"OPTIONS\n"                                                                                                    \
	"    -h, --help\n"                                                                                             \
	"        Display this help message and exit.\n"                                                                \
	"\n"                                                                                                           \
	"EXAMPLES\n"                                                                                                   \
	"    exec ip addr show\n"                                                                                      \
	"    exec sleep 10\n"                                                                                          \
	"    exec journalctl -xe\n"                                                                                    \
	"    exec --help\n"

/**
 * @brief Handles the 'exec' command, executing a shell command from within the CLI.
 *
 * @param input An array of strings representing the command-line arguments.
 */
void exec_command(char **input);

#endif // !EXEC_H
