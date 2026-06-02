#include "../include/globals.h"
#include "../include/utils.h"

#include <errno.h>
#include <net/if.h>
#include <unistd.h>

int detach_ebpf_program(struct bpf_object *obj, const char *interface_name)
{
    if (!obj)
        return 0;

    int ifindex = if_nametoindex(interface_name);
    if (ifindex == 0)
    {
        print(ERROR, "Invalid interface: %s", interface_name);
        return 1;
    }

    struct bpf_tc_hook hook = {
        .sz = sizeof(hook),
        .ifindex = ifindex,
        .attach_point = attach_point,
    };

    struct bpf_program *prog = bpf_object__find_program_by_name(obj, "packet_handler");
    if (!prog)
    {
        print(ERROR, "Failed to find eBPF program 'packet_handler'");
        return 1;
    }

    int prog_fd = bpf_program__fd(prog);
    if (prog_fd < 0)
    {
        print(ERROR, "Invalid eBPF program fd");
        return 1;
    }

    struct bpf_tc_opts opts = {
        .sz = sizeof(opts),
        .handle = 1,
        .priority = 1,
    };

    int err = bpf_tc_detach(&hook, &opts);
    if (err && err != -ENOENT)
    {
        print(ERROR, "Failed to detach eBPF program");
        return 1;
    }

    hook.attach_point = BPF_TC_INGRESS | BPF_TC_EGRESS;
    if (bpf_tc_hook_destroy(&hook))
    {
        print(ERROR, "Failed to destroy TC hook");
        return 1;
    }

    return 0;
}

int unmount_ebpf_module(struct bpf_object *obj, const char *interface_name)
{
    int ret = detach_ebpf_program(obj, interface_name);

    if (stats_map_fd >= 0)
    {
        close(stats_map_fd);
        stats_map_fd = -1;
    }

    if (obj)
        bpf_object__close(obj);

    ebpf_loaded_program_obj = NULL;
    return ret;
}

int cleanup(void)
{
    if (ebpf_loaded_program_obj)
        return unmount_ebpf_module(ebpf_loaded_program_obj, interface);

    if (stats_map_fd >= 0)
    {
        close(stats_map_fd);
        stats_map_fd = -1;
    }

    return 0;
}