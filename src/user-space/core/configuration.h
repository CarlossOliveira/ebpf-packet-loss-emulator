#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "globals.h"

/**
 * @brief Parses the raw attachment point string and updates the application context with the corresponding attachment
 * flags.
 *
 * @param ctx A pointer to the application context containing the BPF and interface information.
 * @param raw_attach_points A string containing the raw attachment point specifications.
 */
void set_attachment_points(app_context_t *ctx, const char *raw_attach_points);

/**
 * @brief Loads the specified eBPF module, updates the application context with the module information, and loads
 * config keys from the eBPF object if available.
 *
 * @param ctx A pointer to the application context containing the BPF and interface information.
 * @param module_name The name of the eBPF module to load.
 */
void set_module(app_context_t *ctx, char *module_name);

#endif // !CONFIGURATION_H
