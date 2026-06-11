#ifndef CLEANUP_H
#define CLEANUP_H

#include "globals.h"

/**
 * @brief Cleans up resources used by the application, including unloading the BPF module and closing any open file
 * descriptors.
 *
 * @param ctx A pointer to the application context.
 * @return 0 on success, -1 on failure.
 */
int cleanup(app_context_t *ctx);

#endif // !CLEANUP_H
