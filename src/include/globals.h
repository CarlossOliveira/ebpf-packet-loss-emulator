#ifndef GLOBALS_H
#define GLOBALS_H

#define LOG_FILE "logs/packet_loss_emulator.log"
#define BPF_MAP_DIR "/sys/fs/bpf"

#define PACKETS_PROCESSED 0
#define PACKETS_DROPPED 1
#define TOTAL_EXECUTION_TIME 2

#define MAX_CONFIG_ENTRIES 10
#define CONFIG_KEY_SIZE 64
#define BPF_STATE_SIZE 1024

#ifdef APP
#include <bpf/libbpf.h>
#include <linux/bpf.h>
#include <stdatomic.h>
#include <stdbool.h>

extern atomic_bool active;
extern atomic_bool bpf_module_change_requested;
extern atomic_bool stats_dump_requested;
extern char *interface;
extern int attach_point;
extern int stats_map_fd;
extern int config_map_fd;
extern char bpf_module_name[256];
extern struct bpf_object *bpf_loaded_program_obj;

// Setup
int setup(void);
void signal_setup(void);
void signal_handler(int signum);

int open_map(struct bpf_object *obj, char *map_name);
int attach_bpf_program(struct bpf_object *obj, const char *interface);
struct bpf_object *mount_bpf_module(const char *module_name,
                                    const char *interface);

// Cleanup
int detach_bpf_program(struct bpf_object *obj, const char *interface);
int unmount_bpf_module(struct bpf_object *obj, const char *interface);
int cleanup(void);

// Helpers
int input(char *prompt, char *buffer, size_t size);
uint64_t read_stats_map(uint8_t key);
int dump_to_log_file(const char *filename, const char *data);
int dump_stats(void);
#endif // !APP

#ifdef BPF
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
  __uint(max_entries, MAX_CONFIG_ENTRIES);
  __type(key, char[CONFIG_KEY_SIZE]);
  __type(value, __u64);
} config_map SEC(".maps");

typedef struct {
  __u8 data[BPF_STATE_SIZE];
} bpf_state_t;
struct {
  __uint(type, BPF_MAP_TYPE_HASH);
  __uint(max_entries, 1);
  __type(key, char[CONFIG_KEY_SIZE]);
  __type(value, bpf_state_t);
} bpf_module_memory SEC(".maps");

#endif // !BPF

#endif // !GLOBALS_H