#include "include/globals.h"
#include "include/utils.h"

#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <net/if.h>

static void wait_for_enter(void);

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
    {
        print(ERROR, "Usage: %s <interface> [ingress|egress]", argv[0]);
        return 1;
    }

    interface = argv[1];

    if (argc == 3)
    {
        lower(argv[2]);
        if (strcmp(argv[2], "egress") == 0)
            attach_point = BPF_TC_EGRESS;
        else if (strcmp(argv[2], "ingress") == 0)
            attach_point = BPF_TC_INGRESS;
        else
        {
            print(ERROR, "Invalid attach point: %s. Use 'ingress' or 'egress'.", argv[2]);
            return 1;
        }
    }

    if (setup() != 0)
        return 1;

    while (atomic_load(&active))
    {
        print(NULL, "Press Ctrl+C to exit, Ctrl+\\ to dump stats.");
        print(NULL, "Available eBPF modules:");
        list_dir(BPF_MODULES_DIR, ".bpf.o");

        print(NULL, "Please select one of the available behaviors: ");
        char choice[256];
        if (fgets(choice, sizeof(choice), stdin) == NULL)
        {
            if (!atomic_load(&active))
                goto EOP;
            print(ERROR, "Error reading input");
            continue;
        }
        choice[strcspn(choice, "\n")] = 0;

        if (choice[0] == '\0')
            continue;

        ebpf_loaded_program_obj = mount_ebpf_module(choice, interface);
        if (!ebpf_loaded_program_obj)
        {
            print(ERROR, "Failed to attach module: %s", choice);
            continue;
        }

        print(NULL, "\n");
        print(SUCCESS, "Module %s attached successfully", choice);
        print(NULL, "Press Enter to change the module...");
        wait_for_enter();

    EOP:
        print(NULL, "detach ifindex=%d attach_point=%d handle=%d priority=%d",
              if_nametoindex(interface), attach_point, 1, 1);
        if (unmount_ebpf_module(ebpf_loaded_program_obj, interface) != 0)
            print(ERROR, "Failed to detach module: %s", choice);
        else
            print(SUCCESS, "Module %s detached successfully", choice);
    }

    dump_stats_to_log_file();
    if (cleanup() != 0)
    {
        print(ERROR, "Cleanup failed");
        return 1;
    }
    print(SUCCESS, "exited(0)\n");

    return 0;
}

static void wait_for_enter(void)
{
    int c;

    clearerr(stdin);

    while (1)
    {
        c = getchar();

        if (c == '\n')
            return;

        if (c == EOF)
        {
            if (errno == EINTR && atomic_load(&active))
            {
                clearerr(stdin);
                continue;
            }
            else if (!atomic_load(&active))
                return;

            clearerr(stdin);
            continue;
        }
    }
}