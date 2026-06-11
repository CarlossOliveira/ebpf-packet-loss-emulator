#ifndef CLI_H
#define CLI_H

#include "globals.h"

/**
 * @brief Starts the command-line interface (CLI) for the application, allowing the user to interact with the loaded BPF
 * module and view statistics.
 *
 * @param ctx A pointer to the application context containing the BPF and interface information.
 */
void cli(app_context_t *ctx);

/**
 * @brief Handles user input from the CLI, parsing commands and executing the corresponding actions.
 *
 * @param ctx A pointer to the application context containing the BPF and interface information.
 * @param input The user input string.
 */
void handle_input(app_context_t *ctx, const char *input);

/**
 * @brief Prompts the user for input and reads a line of text from the console. Creates a history of entered commands
 * and supports basic line editing.
 *
 * @param prompt The prompt message to display to the user.
 * @param buffer A pointer to a character array where the input will be stored.
 * @param size The size of the buffer in bytes.
 * @return 0 on success, -1 on failure.
 */
int input(char *prompt, char *buffer, size_t size);

#endif // !CLI_H
