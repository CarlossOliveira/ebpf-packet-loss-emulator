#ifndef BPF_UTILS_H
#define BPF_UTILS_H

#include "globals.h"
#include <bpf/bpf_helpers.h>

static __always_inline long update_stats(__u32 key, __u64 value)
{
    return bpf_map_update_elem(&stats_map, &key, &value, BPF_ANY);
}

static __always_inline __u64 get_stats(__u32 key)
{
    __u64 *value = (__u64 *)bpf_map_lookup_elem(&stats_map, &key);

    if (!value)
        return 0;

    return *value;
}

#endif // !BPF_UTILS_H