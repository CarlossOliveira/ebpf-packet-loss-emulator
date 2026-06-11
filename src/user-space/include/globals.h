/**
 * @brief Global variables for the user-space application.
 */
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

/**
 * @brief Structure to hold runtime events that can be triggered by the CLI or other parts of the application, such as
 * module switching or shutdown requests.
 */
typedef struct {
	atomic_bool module_switch_requested; /**< Flag indicating a module switch request. */
	atomic_bool shutdown_requested;	     /**< Flag indicating a shutdown request. */
} runtime_events_t;

/**
 * @brief Structure to hold file descriptors for the BPF maps.
 */
typedef struct {
	int stats_map_fd;  /**< File descriptor for the statistics map. */
	int config_map_fd; /**< File descriptor for the configuration map. */
} maps_t;

/**
 * @brief Structure to hold the BPF context information.
 */
typedef struct {
	maps_t maps;					       /**< BPF maps. */
	char module_name[256];				       /**< Module name. */
	char config_keys[MAX_CONFIG_ENTRIES][CONFIG_KEY_SIZE]; /**< BPF configuration keys. */
	struct bpf_object *object;			       /**< BPF object. */
} bpf_context_t;

/**
 * @brief Structure to hold interface context information.
 */
typedef struct {
	char name[IFNAMSIZ];  /**< Interface name. */
	unsigned int ifindex; /**< Interface index. */
	int tc_attach_points; /**< Traffic control attachment points. */
	int xdp_attach_point; /**< XDP attachment point. */

	char default_interface_name[IFNAMSIZ]; /**< Default interface name. */
	int default_tc_attach_points;	       /**< Default traffic control attachment points. */
	int default_xdp_attach_point;	       /**< Default XDP attachment point. */
} interface_context_t;

/**
 * @brief Structure to hold the application context information.
 */
typedef struct {
	bpf_context_t bpf;	       /**< BPF-related context and state. */
	interface_context_t interface; /**< Interface-related context and state. */
} app_context_t;

extern runtime_events_t *runtime_events;
extern app_context_t *app_ctx;

#endif // !USER_SPACE_GLOBALS_H
