#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <stddef.h>

/**
 * @brief Gets the current local time as a formatted string.
 *
 * @param buffer A pointer to a character array where the formatted time string will be stored.
 * @param size The size of the buffer in bytes.
 * @return A pointer to the buffer containing the formatted time string, or NULL on failure.
 */
char *get_current_time(char *buffer, size_t size);

#endif // !TIME_UTILS_H
