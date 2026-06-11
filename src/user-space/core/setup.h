#ifndef SETUP_H
#define SETUP_H

#include "globals.h"

/**
 * @brief Initializes the application context, including setting up the BPF and interface contexts.
 *
 * @return A pointer to the initialized application context, or NULL on failure.
 */
app_context_t *setup(void);

#endif // !SETUP_H
