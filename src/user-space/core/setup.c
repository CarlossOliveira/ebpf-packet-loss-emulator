#include "setup.h"

#include "globals.h"

#include "bpf_manager.h"

#include "elf_utils.h"
#include "io_utils.h"

#include <bpf/libbpf.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <linux/if_link.h>
#include <net/if.h>

static void signal_setup(void);
static void signal_handler(int signum);

app_context_t *setup(void)
{
	// Signal handling setup
	signal_setup();
	rl_catch_signals = 0; // Disable readline's default signal handling

#ifndef DEBUG
	libbpf_set_print(NULL);
#endif

	// App context initialization
	app_context_t *ctx = calloc(1, sizeof(app_context_t));
	if (!ctx) {
		print(ERROR, "Failed to allocate app context");
		return NULL;
	}

	ctx->bpf.maps.stats_map_fd = -1;
	ctx->bpf.maps.config_map_fd = -1;

	// Set default values for interface context
	ctx->interface.default_tc_attach_points = BPF_TC_INGRESS;
	ctx->interface.default_xdp_attach_point = XDP_FLAGS_SKB_MODE;

	// Initialize runtime_events
	runtime_events = calloc(1, sizeof(runtime_events_t));
	if (!runtime_events) {
		print(ERROR, "Failed to allocate runtime_events");
		free(ctx);
		return NULL;
	}
	atomic_store(&runtime_events->shutdown_requested, false);
	atomic_store(&runtime_events->module_switch_requested, false);

	// Compile eBPF modules at startup to ensure they're ready when the user tries to load them
	pid_t pid = fork();
	if (pid == -1) {
		print(ERROR, "Failed to fork process");
		free(ctx);
		return NULL;
	}
	if (pid == 0) {
		// Redirect stdout to /dev/null to suppress make output
		int devnull = open("/dev/null", O_WRONLY);
		if (devnull >= 0) {
			dup2(devnull, STDOUT_FILENO);
			close(devnull);
		}

		char *args[] = {"make", "-C", PROJECT_ROOT, "bpf", NULL};
		execvp("make", args);
		print(ERROR, "Failed to execute make");
		exit(0);
	}

	int status = 0;
	if (waitpid(pid, &status, 0) < 0) {
		print(ERROR, "Failed to wait for BPF build process");
		free(ctx);
		return NULL;
	}

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
		print(ERROR, "Failed to compile eBPF modules");
		free(ctx);
		return NULL;
	}

	return ctx;
}

static void signal_setup(void)
{
	struct sigaction sa = {0};

	sa.sa_handler = signal_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGQUIT, &sa, NULL) == -1 ||
	    sigaction(SIGTERM, &sa, NULL) == -1) {
		print(ERROR, "Failed to set signal handler");
	}
}

static void signal_handler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM) {
		atomic_store(&runtime_events->shutdown_requested, true);
	} else if (signum == SIGQUIT && !atomic_load(&runtime_events->module_switch_requested)) {
		atomic_store(&runtime_events->module_switch_requested, true);
	}

	// Clear the current input line and exit readline loop
	rl_replace_line("", 0);
	rl_done = 1;
}
