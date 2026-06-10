#ifndef USER_SPACE_GLOBALS_H
#define USER_SPACE_GLOBALS_H

#include <bpf/libbpf.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <net/if.h>

#include "common.h"

#define LOG_FILE "logs/packet_loss_emulator.log"

// Command definitions
#define LIST_MODULES_COMMAND "list"
#define LOAD_MODULE_COMMAND "load"
#define UNLOAD_MODULE_COMMAND "unload"
#define CHANGE_CONFIG_COMMAND "config"
#define STATS_COMMAND "stats"
#define HELP_COMMAND "help"
#define CLEAR_SCREEN_COMMAND "clear"
#define EXIT_COMMAND "exit"

typedef struct {
	atomic_bool module_switch_requested;
	atomic_bool shutdown_requested;
} runtime_events_t;

typedef struct {
	int stats_map_fd;
	int config_map_fd;
} maps_t;

typedef struct {
	maps_t maps;
	char module_name[256];
	char config_keys[MAX_CONFIG_ENTRIES][CONFIG_KEY_SIZE];
	struct bpf_object *object;
} bpf_context_t;

typedef struct {
	char name[IFNAMSIZ];
	unsigned int ifindex;
	int tc_attach_points;
	int xdp_attach_point;

	char default_interface_name[IFNAMSIZ];
	int default_tc_attach_points;
	int default_xdp_attach_point;
} interface_context_t;

typedef struct {
	bpf_context_t bpf;
	interface_context_t interface;
} app_context_t;

extern runtime_events_t *runtime_events;
extern app_context_t *app_ctx;

#endif // !USER_SPACE_GLOBALS_H
