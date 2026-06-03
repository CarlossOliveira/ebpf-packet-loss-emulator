#include "../include/globals.h"

#ifdef APP
atomic_bool active = true;
atomic_bool bpf_module_change_requested = false;
atomic_bool stats_dump_requested = false;

char *interface = NULL;
int attach_point = BPF_TC_INGRESS;
int stats_map_fd = -1;
int config_map_fd = -1;
char bpf_module_name[256] = {0};
struct bpf_object *bpf_loaded_program_obj = NULL;
#endif // !APP