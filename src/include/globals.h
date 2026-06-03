#ifndef GLOBALS_H
#define GLOBALS_H

#define LOG_FILE "packet_loss_emulator.log"
#define BPF_MODULES_DIR "../build/modules"
#define BPF_MAP_DIR "/sys/fs/bpf"

#define PACKETS_PROCESSED 0
#define PACKETS_DROPPED 1
#define TOTAL_EXECUTION_TIME 2

#define CONFIG_KEY_SIZE 64

#define DEBUG

#ifdef APP
#include <stdatomic.h>
#include <stdbool.h>
#include <bpf/libbpf.h>
#include <linux/bpf.h>

extern atomic_bool active;
extern atomic_bool bpf_module_change_requested;
extern atomic_bool stats_dump_requested;
extern char *interface;
extern int attach_point;
extern int stats_map_fd;
extern int config_map_fd;
extern char bpf_module_name[256];
extern struct bpf_object *bpf_loaded_program_obj;

int setup(void);
void signal_setup(void);
void signal_handler(int signum);

int attach_bpf_program(struct bpf_object *obj, const char *interface);
struct bpf_object *mount_bpf_module(const char *module_name, const char *interface);
int open_map(struct bpf_object *obj, char *map_name);

int detach_bpf_program(struct bpf_object *obj, const char *interface);
int unmount_bpf_module(struct bpf_object *obj, const char *interface);
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
    __type(key, __u8);
    __type(value, __u64);
} stats_map SEC(".maps");

struct
{
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 10);
    __type(key, char[CONFIG_KEY_SIZE]);
    __type(value, __u64);
} config_map SEC(".maps");
#endif // !BPF

#endif // !GLOBALS_H