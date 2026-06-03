#include "../include/globals.h"
#include "../include/utils.h"

#include <errno.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <readline/readline.h>

int attach_bpf_program(struct bpf_object *obj, const char *interface_name)
{
    if (!obj)
    {
        print(ERROR, "Invalid eBPF object");
        return -1;
    }

    int ifindex = if_nametoindex(interface_name);
    if (ifindex == 0)
    {
        print(ERROR, "Invalid interface: %s", interface_name);
        return -1;
    }

    int points[2];
    int n = 0;

    if (attach_point & BPF_TC_INGRESS)
        points[n++] = BPF_TC_INGRESS;

    if (attach_point & BPF_TC_EGRESS)
        points[n++] = BPF_TC_EGRESS;

    if (n == 0)
    {
        print(ERROR, "No valid attach point specified");
        return -1;
    }

    for (int i = 0; i < n; i++)
    {
        struct bpf_tc_hook hook = {
            .sz = sizeof(hook),
            .ifindex = ifindex,
            .attach_point = points[i],
        };

        int err = bpf_tc_hook_create(&hook);
        if (err && err != -EEXIST)
        {
            print(ERROR, "Failed to create TC hook");
            return -1;
        }

        struct bpf_program *prog = bpf_object__find_program_by_name(obj, "packet_handler");

        if (!prog)
        {
            print(ERROR, "Failed to find eBPF program 'packet_handler'");
            return -1;
        }

        int prog_fd = bpf_program__fd(prog);
        if (prog_fd < 0)
        {
            print(ERROR, "Invalid eBPF program fd");
            return -1;
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
            return -1;
        }
    }

    return n;
}

struct bpf_object *mount_bpf_module(const char *module_name, const char *interface_name)
{
    char path[512];

    if (strstr(module_name, ".bpf.o"))
        snprintf(path, sizeof(path), "%s/%s", BPF_MODULES_DIR, module_name);
    else
        snprintf(path, sizeof(path), "%s/%s.bpf.o", BPF_MODULES_DIR, module_name);

    struct bpf_object *obj = bpf_object__open_file(path, NULL);
    int err = libbpf_get_error(obj);

    if (err)
    {
        print(ERROR, "Failed to open eBPF object file: %s", path);
        return NULL;
    }

    err = bpf_object__load(obj);
    if (err)
    {
        errno = err < 0 ? -err : err;
        print(ERROR, "Failed to load eBPF object file: %s", path);
        bpf_object__close(obj);
        return NULL;
    }

    int new_stats_map_fd = open_map(obj, "stats_map");
    if (new_stats_map_fd < 0)
    {
        print(ERROR, "Failed to open stats map");
        bpf_object__close(obj);
        return NULL;
    }

    int new_config_map_fd = open_map(obj, "config_map");
    if (new_config_map_fd < 0)
    {
        print(ERROR, "Failed to open config map");
        bpf_object__close(obj);
        return NULL;
    }

    if (attach_bpf_program(obj, interface_name) < 0)
    {
        print(ERROR, "Failed to attach eBPF program");
        bpf_object__close(obj);
        return NULL;
    }

    stats_map_fd = new_stats_map_fd;
    config_map_fd = new_config_map_fd;

    return obj;
}

int open_map(struct bpf_object *obj, char *map_name)
{
    struct bpf_map *map = bpf_object__find_map_by_name(obj, map_name);
    if (!map)
    {
        print(ERROR, "Failed to find map '%s' in eBPF object", map_name);
        return -1;
    }

    return bpf_map__fd(map);
}

int setup(void)
{
    signal_setup();

    rl_catch_signals = 0; // Disable readline's default signal handling

    pid_t pid = fork();
    if (pid == -1)
    {
        print(ERROR, "Failed to fork process");
        return 1;
    }

    if (pid == 0)
    {
        char *args[] = {"make", "bpf", "-f", "../Makefile", NULL};
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
    {
        atomic_store(&active, false);
    }
    else if (signum == SIGQUIT && !atomic_load(&bpf_module_change_requested))
    {
        atomic_store(&bpf_module_change_requested, true);
    }

    // Clear the current input line and exit readline loop
    rl_replace_line("", 0);
    rl_done = 1;
}