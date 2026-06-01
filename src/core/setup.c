#include "../include/globals.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/wait.h>

#include <signal.h>

extern volatile bool active;

// Attach eBPF module to the specified interface using tc
int attach_module(const char *filename, const char *interface)
{
    // Implementation for attaching eBPF module
    return 0;
}

void signal_handler(int signum)
{
    (void)signum;
    // Handle SIGINT for graceful shutdown
    active = 0;
}

int setup()
{
    struct sigaction sa = {0};
    sigset_t set;

    sigfillset(&set);
    sigdelset(&set, SIGINT);

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1)
    {
        print("Failed to set signal mask", ERROR);
        return 1;
    }

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // sem SA_RESTART, para desbloquear fgets/read

    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        print("Failed to set signal handler", ERROR);
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        print("Failed to fork process", ERROR);
        return 1;
    }
    else if (pid == 0)
    {
        char *args[] = {
            "make",
            "ebpf",
            NULL};

        execv("/usr/bin/make", args);
    }

    wait(NULL); // Wait for the child process to finish
    return 0;
}