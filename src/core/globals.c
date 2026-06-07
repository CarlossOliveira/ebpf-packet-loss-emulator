#include "../include/globals.h"

#ifdef APP
#include <linux/if_link.h>

atomic_bool active = true;
atomic_bool bpf_module_change_requested = false;
atomic_bool stats_dump_requested = false;

char *interface = NULL;
int attach_point = BPF_TC_INGRESS | XDP_FLAGS_SKB_MODE;
int stats_map_fd = -1;
int config_map_fd = -1;
char bpf_module_name[256] = {0};
char bpf_config_keys[MAX_CONFIG_ENTRIES][CONFIG_KEY_SIZE] = {0};
struct bpf_object *bpf_loaded_program_obj = NULL;
#endif // !APP