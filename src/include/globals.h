#ifndef GLOBALS_H
#define GLOBALS_H

#define LOG_FILE "packet_loss_emulator.log"
#define BPF_MODULES_DIR "../modules"
#define BPF_MAP_DIR "/sys/fs/bpf"

#define PACKETS_PROCESSED 0
#define PACKETS_DROPPED 1
#define TOTAL_EXECUTION_TIME 2

#ifdef APP
#include <stdatomic.h>
#include <stdbool.h>
#include <bpf/libbpf.h>
#include <linux/bpf.h>

extern atomic_bool active;
extern char *interface;
extern int attach_point;
extern int stats_map_fd;
extern char bpf_module_name[256];
extern struct bpf_object *ebpf_loaded_program_obj;

int setup(void);
void signal_setup(void);
void signal_handler(int signum);

struct bpf_tc_hook attach_ebpf_program(struct bpf_object *obj, const char *interface);
struct bpf_object *mount_ebpf_module(const char *module_name, const char *interface);
int open_stats_map(struct bpf_object *obj);

int detach_ebpf_program(struct bpf_object *obj, const char *interface);
int unmount_ebpf_module(struct bpf_object *obj, const char *interface);
int cleanup(void);
#endif // !APP

#ifdef BPF
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>

#define TC_ACT_OK 0
#define TC_ACT_SHOT 2

struct
{
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 3);
    __type(key, __u32);
    __type(value, __u64);
} stats_map SEC(".maps");
#endif // !BPF

#endif // !GLOBALS_H