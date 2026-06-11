#ifndef LOADER_H
#define LOADER_H

#include "globals.h"

/**
 * @brief Opens a BPF map from the given eBPF object and map name.
 *
 * @param obj The eBPF object containing the map.
 * @param map_name The name of the map to open.
 * @return The file descriptor of the opened map, or -1 on failure.
 */
int open_map(struct bpf_object *obj, char *map_name);

/**
 * @brief Loads an eBPF module based on the module name specified in the application context, attaches it to the
 * specified interface, and opens the necessary maps.
 *
 * @param ctx A pointer to the application context containing the BPF and interface information.
 * @return A pointer to the loaded eBPF object on success, or NULL on failure.
 */
struct bpf_object *load_bpf_module(app_context_t *ctx);

/**
 * @brief Unloads the currently loaded eBPF module, detaches it from the interface, and closes any open map file
 * descriptors.
 *
 * @param ctx A pointer to the application context containing the BPF and interface information.
 * @return 0 on success, -1 on failure.
 */
int unload_bpf_module(app_context_t *ctx);

#endif // !LOADER_H
