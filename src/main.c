#include "include/globals.h"
#include "include/utils.h"

#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <net/if.h>

static void wait_for_enter(void);
static int check_sudo(void);

int main(int argc, char *argv[])
{
    if (check_sudo() != 0)
        return 1;

    if (argc < 2 || argc > 3)
    {
        print(ERROR, "Usage: %s <interface> [<attach_point>]", argv[0]);
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
        else if ((strcmp(argv[2], "ingress|egress") == 0) || (strcmp(argv[2], "egress|ingress") == 0))
            attach_point = BPF_TC_INGRESS | BPF_TC_EGRESS;
        else
        {
            print(ERROR, "Invalid attach point: %s. Use 'ingress', 'egress', or 'ingress|egress'.", argv[2]);
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

        // Copy the selected module name to the global variable for logging purposes
        strncpy(bpf_module_name, choice, sizeof(bpf_module_name) - 1);
        bpf_module_name[sizeof(bpf_module_name) - 1] = '\0';

        ebpf_loaded_program_obj = mount_ebpf_module(choice, interface);
        if (ebpf_loaded_program_obj == NULL)
        {
            print(ERROR, "Failed to attach module: %s", choice);
            continue;
        }

        print(NULL, "\n");
        print(SUCCESS, "Module %s attached successfully", choice);
        print(NULL, "Press Enter to change the module...");
        wait_for_enter();

    EOP:
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

static int check_sudo(void)
{
    if (geteuid() != 0)
    {
        print(ERROR, "This program must be run as root.");
        return 1;
    }
    return 0;
}