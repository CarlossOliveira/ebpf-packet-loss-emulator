#include "string_utils.h"

#include <ctype.h>  // tolower
#include <stddef.h> // size_t
#include <stdlib.h> // calloc, free
#include <string.h> // strdup, strtok
#include <stdarg.h> // va_list, va_start, va_end

void lower(char *str)
{
	if (!str)
		return;

	for (size_t i = 0; str[i] != '\0'; i++)
		str[i] = (char)tolower((unsigned char)str[i]);
}

int count_char(const char *str, char c)
{
	int count = 0;

	if (!str)
		return 0;

	while (*str) {
		if (*str == c)
			count++;
		str++;
	}

	return count;
}

void remove_char(char *str, char c)
{
	if (!str)
		return;

	char *src = str, *dst = str;

	while (*src) {
		if (*src != c)
			*dst++ = *src;
		src++;
	}
	*dst = '\0';
}

char **strsplit(const char *str, char delim, int max_parts)
{
	if (!str || max_parts < 0)
		return NULL;

	if (max_parts == 0)
		max_parts = count_char(str, delim) + 1;

	char **result = calloc((size_t)max_parts + 1, sizeof(char *));
	if (!result)
		return NULL;

	char *copy = strdup(str);
	if (!copy) {
		free(result);
		return NULL;
	}

	char *start = copy;
	int index = 0;

	while (*start == delim)
		start++;

	while (*start && index < max_parts - 1) {
		char *pos = strchr(start, delim);

		if (!pos)
			break;

		*pos = '\0';

		result[index] = strdup(start);
		if (!result[index]) {
			strsplit_free(result);
			free(copy);
			return NULL;
		}

		index++;

		start = pos + 1;

		while (*start == delim)
			start++;
	}

	if (*start) {
		result[index] = strdup(start);
		if (!result[index]) {
			strsplit_free(result);
			free(copy);
			return NULL;
		}
		index++;
	}

	result[index] = NULL;

	free(copy);
	return result;
}

void strsplit_free(char **parts)
{
	if (!parts)
		return;

	for (size_t i = 0; parts[i] != NULL; i++)
		free(parts[i]);
	free(parts);
}
