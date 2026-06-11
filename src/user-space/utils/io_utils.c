#include "io_utils.h"

#include <errno.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

void print(const char *code, const char *msg, ...)
{
	if (!msg || !msg[0])
		return;

	const char *color = "";
	if (code) {
		if (strcmp(code, WARNING) == 0)
			color = YELLOW;
		else if (strcmp(code, ERROR) == 0)
			color = RED;
		else if (strcmp(code, CAUTION) == 0)
			color = BLUE;
		else if (strcmp(code, SUCCESS) == 0)
			color = GREEN;
	}

	if (code && code[0] && color[0])
		printf("%s%s[%s]%s ", color, BOLD, code, RESET);
	else if (code && code[0])
		printf("[%s] ", code);

	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);

	if (errno != 0 && code && strcmp(color, RED) == 0) // Only print the error string for ERROR messages
		printf(" (%s)", strerror(errno));
	printf("\n");
	errno = 0;
}

void wait_for_char(char expected_char, atomic_bool *active)
{
	while (1) {
		errno = 0;
		int c = getchar();

		if (c == expected_char)
			return;

		if (c == EOF) {
			if (errno == EINTR && atomic_load(active)) {
				clearerr(stdin);
				continue;
			}
			if (!atomic_load(active))
				return;

			clearerr(stdin);
			continue;
		}
	}
}

int make_path(const char *path)
{
	if (!path || !*path)
		return -1;

	char tmp_path[1024];

	if (snprintf(tmp_path, sizeof(tmp_path), "%s", path) >= (int)sizeof(tmp_path))
		return -1;

	size_t len = strlen(tmp_path);

	/* Remove trailing '/' except for root */
	if (len > 1 && tmp_path[len - 1] == '/')
		tmp_path[len - 1] = '\0';

	for (char *p = tmp_path + 1; *p; p++) {
		if (*p == '/') {
			*p = '\0';

			if (mkdir(tmp_path, 0755) != 0 && errno != EEXIST)
				return -1;

			*p = '/';
		}
	}

	if (mkdir(tmp_path, 0755) != 0 && errno != EEXIST)
		return -1;

	return 0;
}
