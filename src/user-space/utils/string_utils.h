#ifndef STRING_UTILS_H
#define STRING_UTILS_H

/**
 * @brief Converts a string to lowercase.
 *
 * @param str The string to convert.
 */
void lower(char *str);

/**
 * @brief Counts the occurrences of a character in a string.
 *
 * @param str The string to search.
 * @param c The character to count.
 * @return The number of occurrences of the character.
 */
int count_char(const char *str, char c);

/**
 * @brief Removes all occurrences of a character from a string.
 *
 * @param str The string to modify.
 * @param c The character to remove.
 */
void remove_char(char *str, char c);

/**
 * @brief Splits a string into parts based on a delimiter.
 *
 * @param str The string to split.
 * @param delim The delimiter character.
 * @param max_parts The maximum number of parts to create.
 * @return A pointer to an array of strings containing the split parts, or NULL on failure.
 */
char **strsplit(const char *str, char delim, int max_parts);

/**
 * @brief Frees the memory allocated for the parts array returned by strsplit.
 *
 * @param parts The array of strings to free.
 */
void strsplit_free(char **parts);

#endif // !STRING_UTILS_H
