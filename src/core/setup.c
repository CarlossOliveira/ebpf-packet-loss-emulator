#include "globals.h"

#include "elf_utils.h"
#include "io_utils.h"

#include <bpf/libbpf.h>
#include <linux/bpf.h>
#include <errno.h>
#include <linux/if_link.h>
#include <net/if.h>
#include <readline/readline.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int setup(void)
{
	signal_setup();

	rl_catch_signals = 0; // Disable readline's default signal handling

	pid_t pid = fork();
	if (pid == -1) {
		print(ERROR, "Failed to fork process");
		return 1;
	}

	if (pid == 0) {
		char *args[] = {"make", "-C", PROJECT_ROOT, "bpf", NULL};
		execvp("make", args);
		print(ERROR, "Failed to execute make");
		exit(0);
	}

	int status = 0;
	if (waitpid(pid, &status, 0) < 0) {
		print(ERROR, "Failed to wait for BPF build process");
		return 1;
	}

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
		print(ERROR, "Failed to compile eBPF modules");
		return 1;
	}

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
	    sigaction(SIGTERM, &sa, NULL) == -1) {
		print(ERROR, "Failed to set signal handler");
	}
}

void signal_handler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM) {
		atomic_store(&active, false);
	} else if (signum == SIGQUIT &&
		   !atomic_load(&bpf_module_change_requested)) {
		atomic_store(&bpf_module_change_requested, true);
	}

	// Clear the current input line and exit readline loop
	rl_replace_line("", 0);
	rl_done = 1;
}

int open_map(struct bpf_object *obj, char *map_name)
{
	struct bpf_map *map = bpf_object__find_map_by_name(obj, map_name);
	if (!map) {
		print(ERROR, "Failed to find map '%s' in eBPF object",
		      map_name);
		return -1;
	}

	return bpf_map__fd(map);
}

int attach_bpf_program(struct bpf_object *obj, const char *interface_name)
{
	if (!obj) {
		print(ERROR, "Invalid eBPF object");
		return -1;
	}

	int ifindex = if_nametoindex(interface_name);
	if (ifindex == 0) {
		print(ERROR, "Invalid interface: %s", interface_name);
		return -1;
	}

	// Determine program type by checking for known sections in the ELF file
	char bpf_elf_filename[512];
	snprintf(bpf_elf_filename, sizeof(bpf_elf_filename), "%s/%s.bpf.o",
		 BPF_OBJECT_DIR, bpf_module_name);
	int elf_fd = open_elf_file(bpf_elf_filename);
	if (elf_fd < 0) {
		print(ERROR,
		      "Failed to open eBPF object file for inspection: %s",
		      bpf_elf_filename);
		return -1;
	}

	bool module_is_tc = elf_has_section(elf_fd, "classifier");
	bool module_is_xdp = elf_has_section(elf_fd, "xdp");
	close_elf_file(elf_fd);

	// Load the program fd
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

	// Attach based on program type
	int n = 0;
	if (module_is_tc) {
		int points[2];

		if (attach_point & BPF_TC_INGRESS)
			points[n++] = BPF_TC_INGRESS;

		if (attach_point & BPF_TC_EGRESS)
			points[n++] = BPF_TC_EGRESS;

		if (n == 0) {
			print(ERROR,
			      "No valid attach point specified for TC module");
			return -1;
		}
		for (int i = 0; i < n; i++) {
			struct bpf_tc_hook hook = {
			    .sz = sizeof(hook),
			    .ifindex = ifindex,
			    .attach_point = points[i],
			};

			int err = bpf_tc_hook_create(&hook);
			if (err && err != -EEXIST) {
				print(ERROR, "Failed to create hook");
				n--;
				continue;
			}

			struct bpf_tc_opts opts = {
			    .sz = sizeof(opts),
			    .prog_fd = prog_fd,
			    .handle = 1,
			    .priority = 1,
			};

			err = bpf_tc_attach(&hook, &opts);
			if (err) {
				print(ERROR, "Failed to attach eBPF program");
				n--;
				continue;
			}
		}
	} else if (module_is_xdp) {
		int flags =
		    (attach_point &
		     ~(BPF_TC_INGRESS | BPF_TC_EGRESS)); // Extract XDP flags
		if (bpf_set_link_xdp_fd(ifindex, prog_fd, flags) <
		    0) { // bpf_xdp_attach doesn't work, idk :/
			print(ERROR, "Failed to attach XDP program");
			return -1;
		}
		n = 1;
	} else {
		print(ERROR, "Unknown eBPF program type");
		return -1;
	}

	return n;
}

struct bpf_object *mount_bpf_module(const char *module_name,
				    const char *interface_name)
{
	char path[512];

	if (strstr(module_name, ".bpf.o"))
		snprintf(path, sizeof(path), "%s/%s", BPF_OBJECT_DIR,
			 module_name);
	else
		snprintf(path, sizeof(path), "%s/%s.bpf.o", BPF_OBJECT_DIR,
			 module_name);

	struct bpf_object *obj = bpf_object__open_file(path, NULL);
	int err = libbpf_get_error(obj);

	if (err) {
		print(ERROR, "Failed to open eBPF object file: %s", path);
		return NULL;
	}

	err = bpf_object__load(obj);
	if (err) {
		errno = err < 0 ? -err : err;
		print(ERROR, "Failed to load eBPF object file: %s", path);
		bpf_object__close(obj);
		return NULL;
	}

	int new_stats_map_fd = open_map(obj, "stats_map");
	if (new_stats_map_fd < 0) {
		print(ERROR, "Failed to open stats map");
		bpf_object__close(obj);
		return NULL;
	}

	int new_config_map_fd = open_map(obj, "config_map");
	if (new_config_map_fd < 0) {
		print(ERROR, "Failed to open config map");
		bpf_object__close(obj);
		return NULL;
	}

	if (attach_bpf_program(obj, interface_name) <= 0) {
		print(ERROR, "Failed to attach eBPF program");
		bpf_object__close(obj);
		return NULL;
	}

	stats_map_fd = new_stats_map_fd;
	config_map_fd = new_config_map_fd;

	return obj;
}
