#include "string_utils.h"

#include <ctype.h>  // tolower
#include <stddef.h> // size_t
#include <stdlib.h> // calloc, free
#include <string.h> // strdup, strtok

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

char **strsplit(const char *str, char delim)
{
	if (!str)
		return NULL;

	int count = count_char(str, delim) + 1;
	char **result = calloc((size_t)count + 1, sizeof(char *));
	if (!result)
		return NULL;

	char *copy = strdup(str);
	if (!copy) {
		free(result);
		return NULL;
	}

	char delim_str[2] = {delim, '\0'};

	int index = 0;
	char *token = strtok(copy, delim_str);

	while (token) {
		result[index] = strdup(token);
		if (!result[index]) {
			strsplit_free(result);
			free(copy);
			return NULL;
		}
		index++;
		token = strtok(NULL, delim_str);
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