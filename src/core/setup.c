#include "../include/globals.h"
#include "../include/utils.h"

#include <errno.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static atomic_bool stats_dump_requested = false;

struct bpf_tc_hook attach_ebpf_program(struct bpf_object *obj, const char *interface_name)
{
    int ifindex = if_nametoindex(interface_name);
    if (ifindex == 0)
    {
        print(ERROR, "Invalid interface: %s", interface_name);
        return (struct bpf_tc_hook){0};
    }

    struct bpf_tc_hook hook = {
        .sz = sizeof(hook),
        .ifindex = ifindex,
        .attach_point = attach_point,
    };

    int err = bpf_tc_hook_create(&hook);
    if (err && err != -EEXIST)
    {
        print(ERROR, "Failed to create TC hook");
        return (struct bpf_tc_hook){0};
    }

    struct bpf_program *prog = bpf_object__find_program_by_name(obj, "packet_handler");

    if (!prog)
    {
        print(ERROR, "Failed to find eBPF program 'packet_handler'");
        return (struct bpf_tc_hook){0};
    }

    int prog_fd = bpf_program__fd(prog);
    if (prog_fd < 0)
    {
        print(ERROR, "Invalid eBPF program fd");
        return (struct bpf_tc_hook){0};
    }

    struct bpf_tc_opts opts = {
        .sz = sizeof(opts),
        .prog_fd = prog_fd,
        .handle = 1,
        .priority = 1,
    };

    err = bpf_tc_attach(&hook, &opts);
    if (err)
    {
        print(ERROR, "Failed to attach eBPF program");
        return (struct bpf_tc_hook){0};
    }

    return hook;
}

struct bpf_object *mount_ebpf_module(const char *module_name, const char *interface_name)
{
    char path[512];

    if (strstr(module_name, ".bpf.o"))
        snprintf(path, sizeof(path), "%s/%s", BPF_MODULES_DIR, module_name);
    else
        snprintf(path, sizeof(path), "%s/%s.bpf.o", BPF_MODULES_DIR, module_name);

    struct bpf_object *obj = bpf_object__open_file(path, NULL);
    if (!obj)
    {
        print(ERROR, "Failed to open eBPF object file: %s", path);
        return NULL;
    }

    int err = bpf_object__load(obj);
    if (err)
    {
        print(ERROR, "Failed to load eBPF object: %s", path);
        bpf_object__close(obj);
        return NULL;
    }

    if (attach_ebpf_program(obj, interface_name).ifindex == 0)
    {
        print(ERROR, "Failed to attach eBPF program");
        bpf_object__close(obj);
        return NULL;
    }

    stats_map_fd = open_stats_map(obj);
    if (stats_map_fd < 0)
    {
        print(ERROR, "Failed to open stats map");
        bpf_object__close(obj);
        return NULL;
    }

    return obj;
}

int open_stats_map(struct bpf_object *obj)
{
    struct bpf_map *map = bpf_object__find_map_by_name(obj, "stats_map");
    if (!map)
    {
        print(ERROR, "Failed to find stats_map in eBPF object");
        return -1;
    }

    return bpf_map__fd(map);
}

int setup(void)
{
    signal_setup();

    pid_t pid = fork();
    if (pid == -1)
    {
        print(ERROR, "Failed to fork process");
        return 1;
    }

    if (pid == 0)
    {
        char *args[] = {"make", "ebpf", "-f", "../Makefile", NULL};
        execv("/usr/bin/make", args);
    }
    waitpid(pid, NULL, 0);

    return 0;
}

void signal_setup(void)
{
    struct sigaction sa = {0};

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1 ||
        sigaction(SIGQUIT, &sa, NULL) == -1 ||
        sigaction(SIGTERM, &sa, NULL) == -1)
    {
        print(ERROR, "Failed to set signal handler");
    }
}

void signal_handler(int signum)
{
    if (signum == SIGINT || signum == SIGTERM)
        atomic_store(&active, false);
    else if (signum == SIGQUIT && !atomic_load(&stats_dump_requested))
    {
        atomic_store(&stats_dump_requested, true);
        dump_stats_to_log_file();
        atomic_store(&stats_dump_requested, false);
    }
}