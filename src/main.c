#include "../include/globals.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <signal.h>

volatile bool active = true;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        print(ERROR, "Usage: %s <interface>", argv[0]);
        return 1;
    }

    while (active)
    {
        print(ERROR, "Press Ctrl+C to exit...");
        print(NULL, "Please select one of the following behaviors:");
        list_dir(BPF_MODULES_DIR, ".bpf");

        fflush(stdin);
        char choice[256];
        if (fgets(choice, sizeof(choice), stdin) == NULL)
        {
            print(ERROR, "Error reading input");
            continue;
        }
        choice[strcspn(choice, "\n")] = 0; // Remove newline character

        // Handle user choice and apply corresponding eBPF module
        if (attach_module(choice, argv[1]) != 0)
        {
            print(ERROR, "Failed to attach module: %s", choice);
        }
        else
        {
            print(SUCCESS, "Module %s attached successfully", choice);
            print(NULL, "Press any key to change the module...");
            getchar(); // Wait for any key press to change the module
        }

        if (detach_module(choice, argv[1]) != 0)
        {
            print(ERROR, "Failed to detach module: %s", choice);
        }
        else
        {
            print(SUCCESS, "Module %s detached successfully", choice);
        }
    }

    if (cleanup() != 0)
    {
        print(ERROR, "Cleanup failed");
        return 1;
    }

    return 0;
}