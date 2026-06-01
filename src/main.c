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
        print("Usage: %s <interface>\n", ERROR);
        return 1;
    }

    while (active)
    {
        print("Press Ctrl+C to exit...\n", NULL);
        print("Please select one of the following behaviors:\n", NULL);
        list_dir(BPF_MODULES_DIR, ".bpf");

        fflush(stdin);
        char choice[256];
        if (fgets(choice, sizeof(choice), stdin) == NULL)
        {
            print("Error reading input\n", ERROR);
            continue;
        }
        choice[strcspn(choice, "\n")] = 0; // Remove newline character

        // Handle user choice and apply corresponding eBPF module
        if (attach_module(choice, argv[1]) != 0)
        {
            print("Failed to attach module: %s\n", ERROR);
        }
        else
        {
            print("Module %s attached successfully\n", SUCCESS);
            print("Press any key to change the module...\n", NULL);
            getchar(); // Wait for any key press to change the module
        }

        if (detach_module(choice, argv[1]) != 0)
        {
            print("Failed to detach module: %s\n", ERROR);
        }
        else
        {
            print("Module %s detached successfully\n", SUCCESS);
        }
    }

    if (cleanup() != 0)
    {
        print("Cleanup failed\n", ERROR);
        return 1;
    }

    return 0;
}