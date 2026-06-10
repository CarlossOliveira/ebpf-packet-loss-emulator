#ifndef KERNEL_SPACE_GLOBALS_H
#define KERNEL_SPACE_GLOBALS_H

#include "common.h"

#define MAX_CONFIG_ENTRIES 10
#define CONFIG_KEY_SIZE 64

#include "vmlinux.h"
#include <bpf/bpf_helpers.h>

#define TC_ACT_OK 0
#define TC_ACT_SHOT 2

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__uint(max_entries, 3);
	__type(key, __u8);
	__type(value, __u64);
} stats_map SEC(".maps");

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__uint(max_entries, MAX_CONFIG_ENTRIES + 1); // +1 for sentinel
	__type(key, char[CONFIG_KEY_SIZE]);
	__type(value, __u64);
} config_map SEC(".maps");

#endif // !KERNEL_SPACE_GLOBALS_H
