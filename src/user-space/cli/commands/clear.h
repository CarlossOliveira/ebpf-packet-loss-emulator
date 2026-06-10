#ifndef CLEAR_H
#define CLEAR_H

#define CLEAR_SCREEN_COMMAND "clear"

#define CLEAR_HELP_MESSAGE                                                                                             \
	"NAME\n"                                                                                                       \
	"    clear - Clear the terminal screen\n"                                                                      \
	"\n"                                                                                                           \
	"SYNOPSIS\n"                                                                                                   \
	"    clear [OPTIONS]\n"                                                                                        \
	"\n"                                                                                                           \
	"DESCRIPTION\n"                                                                                                \
	"    Clears all visible terminal output and redraws the prompt.\n"                                             \
	"\n"                                                                                                           \
	"OPTIONS\n"                                                                                                    \
	"    -h, --help\n"                                                                                             \
	"        Display this help message and exit.\n"                                                                \
	"\n"                                                                                                           \
	"EXAMPLES\n"                                                                                                   \
	"    clear\n"                                                                                                  \
	"    clear --help\n"

void clear_screen_command(char **input);

#endif // !CLEAR_H
