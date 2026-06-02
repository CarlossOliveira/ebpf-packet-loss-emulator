#include "../include/globals.h"

#ifdef APP
atomic_bool active = true;
char *interface = NULL;
int attach_point = BPF_TC_INGRESS;
int stats_map_fd = -1;
char bpf_module_name[256] = {0};
struct bpf_object *ebpf_loaded_program_obj = NULL;
#endif // !APP