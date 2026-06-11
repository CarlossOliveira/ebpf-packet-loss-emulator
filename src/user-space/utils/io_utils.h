#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <stdatomic.h>

#define WARNING "WARNING"
#define ERROR "ERROR"
#define CAUTION "CAUTION"
#define SUCCESS "SUCCESS"

#define RED "\033[31m"
#define YELLOW "\033[33m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define BOLD "\033[1m"
#define RESET "\033[0m"

/**
 * @brief Prints a formatted message to the console with an optional code prefix.
 *
 * @param code An optional string to prefix the message (e.g., "ERROR", "WARNING"). Can be NULL.
 * @param msg The message format string (like printf).
 * @param ... Additional arguments for the format string.
 */
void print(const char *code, const char *msg, ...);

/**
 * @brief Waits for a specific character input from the user, with support for interruption.
 *
 * @param expected_char The character to wait for.
 * @param active A pointer to an atomic boolean that indicates whether the waiting should continue. If set to false, the
 * function will return even if the expected character is not received.
 */
void wait_for_char(char expected_char, atomic_bool *active);

/**
 * @brief Creates a directory path if it does not already exist.
 *
 * @param path The directory path to create.
 * @return 0 on success, -1 on failure.
 */
int make_path(const char *path);

#endif // !IO_UTILS_H
