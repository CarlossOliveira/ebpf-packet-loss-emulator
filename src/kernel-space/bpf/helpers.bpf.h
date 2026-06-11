#ifndef HELPERS_BPF_H
#define HELPERS_BPF_H

#include "globals.h"

#include <bpf/bpf_helpers.h>

/**
 * @brief Updates a value in the stats map for a given key.
 *
 * @param key The key for the stats entry to update.
 * @param value The new value to set for the specified key.
 * @return 0 on success, or a negative error code on failure.
 */
static __always_inline long update_stats(__u8 key, __u64 value)
{
	return bpf_map_update_elem(&stats_map, &key, &value, BPF_ANY);
}

/**
 * @brief Retrieves a value from the stats map for a given key.
 *
 * @param key The key for the stats entry to retrieve.
 * @return The value associated with the specified key, or 0 if not found.
 */
static __always_inline __u64 get_stats(__u8 key)
{
	__u64 *value = (__u64 *)bpf_map_lookup_elem(&stats_map, &key);

	if (!value)
		return 0;

	return *value;
}

/**
 * @brief Retrieves a value from the config map for a given key.
 *
 * @param key The key for the config entry to retrieve.
 * @return The value associated with the specified key, or 0 if not found.
 */
static __always_inline __u64 get_config_value(const char key[CONFIG_KEY_SIZE])
{
	__u64 *config_value = (__u64 *)bpf_map_lookup_elem(&config_map, key);

	if (!config_value)
		return 0;

	return *config_value;
}

#endif // !HELPERS_BPF_H
