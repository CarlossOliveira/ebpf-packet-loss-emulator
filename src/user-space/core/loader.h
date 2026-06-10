#ifndef LOADER_H
#define LOADER_H

#include "globals.h"

int open_map(struct bpf_object *obj, char *map_name);
struct bpf_object *load_bpf_module(app_context_t *ctx);
int unload_bpf_module(app_context_t *ctx);

#endif // !LOADER_H
