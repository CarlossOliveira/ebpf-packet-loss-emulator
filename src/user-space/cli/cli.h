#ifndef CLI_H
#define CLI_H

#include "globals.h"

void cli(app_context_t *ctx);
int input(char *prompt, char *buffer, size_t size);

#endif // !CLI_H
