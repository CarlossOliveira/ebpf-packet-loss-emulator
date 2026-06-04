#include "../include/globals.h"

#include "io_utils.h"

#include <errno.h>
#include <net/if.h>
#include <unistd.h>

int detach_bpf_program(struct bpf_object *obj, const char *interface_name) {
  if (!obj) {
    print(ERROR, "Invalid eBPF object");
    return -1;
  }

  int ifindex = if_nametoindex(interface_name);
  if (ifindex == 0) {
    print(ERROR, "Invalid interface: %s", interface_name);
    return -1;
  }

  int points[2];
  int n = 0;

  if (attach_point & BPF_TC_INGRESS)
    points[n++] = BPF_TC_INGRESS;

  if (attach_point & BPF_TC_EGRESS)
    points[n++] = BPF_TC_EGRESS;

  if (n == 0) {
    print(ERROR, "No valid attach point specified");
    return -1;
  }

  struct bpf_program *prog =
      bpf_object__find_program_by_name(obj, "packet_handler");
  if (!prog) {
    print(ERROR, "Failed to find eBPF program 'packet_handler'");
    return -1;
  }

  int prog_fd = bpf_program__fd(prog);
  if (prog_fd < 0) {
    print(ERROR, "Invalid eBPF program fd");
    return -1;
  }

  for (int i = 0; i < n; i++) {
    struct bpf_tc_hook hook = {
        .sz = sizeof(hook),
        .ifindex = ifindex,
        .attach_point = points[i],
    };

    struct bpf_tc_opts opts = {
        .sz = sizeof(opts),
        .handle = 1,
        .priority = 1,
    };

    int err = bpf_tc_detach(&hook, &opts);
    if (err && err != -ENOENT) {
      print(ERROR, "Failed to detach eBPF program");
      return -1;
    }
  }

  struct bpf_tc_hook hook = {
      .sz = sizeof(hook),
      .ifindex = ifindex,
      .attach_point = BPF_TC_INGRESS | BPF_TC_EGRESS,
  };

  int err = bpf_tc_hook_destroy(&hook);
  if (err && err != -ENOENT) {
    print(ERROR, "Failed to destroy TC hook");
    return -1;
  }

  return n;
}

int unmount_bpf_module(struct bpf_object *obj, const char *interface_name) {
  int ret = detach_bpf_program(obj, interface_name);

  stats_map_fd = -1;
  config_map_fd = -1;

  if (obj)
    bpf_object__close(obj);

  bpf_loaded_program_obj = NULL;
  return ret;
}

int cleanup(void) {
  if (bpf_loaded_program_obj)
    return unmount_bpf_module(bpf_loaded_program_obj, interface);

  if (stats_map_fd >= 0) {
    close(stats_map_fd);
    stats_map_fd = -1;
  }

  return 0;
}