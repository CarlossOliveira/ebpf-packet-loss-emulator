#include "../include/globals.h"

#include "elf_utils.h"
#include "io_utils.h"

#include <bpf/libbpf.h>
#include <linux/bpf.h>
#include <errno.h>
#include <linux/if_link.h>
#include <net/if.h>
#include <unistd.h>

int detach_bpf_program(struct bpf_object *obj, const char *interface_name)
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

	// Unattach based on program type
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

		struct bpf_tc_opts opts = {
		    .sz = sizeof(opts),
		    .handle = 1,
		    .priority = 1,
		};

		for (int i = 0; i < n; i++) {
			struct bpf_tc_hook hook = {
			    .sz = sizeof(hook),
			    .ifindex = ifindex,
			    .attach_point = points[i],
			};

			int err = bpf_tc_detach(&hook, &opts);
			if (err && err != -ENOENT) {
				print(ERROR, "Failed to detach eBPF program");
				n--;
				continue;
			}
		}

		// Destroy TC hooks to clean up any remaining state.
		struct bpf_tc_hook hook = {
		    .sz = sizeof(hook),
		    .ifindex = ifindex,
		    .attach_point = BPF_TC_INGRESS,
		};
		int err = bpf_tc_hook_destroy(&hook);
		if (err && err != -ENOENT) {
			print(ERROR, "Failed to destroy TC hook");
			n--;
		}

		hook.attach_point = BPF_TC_EGRESS;
		err = bpf_tc_hook_destroy(&hook);
		if (err && err != -ENOENT) {
			print(ERROR, "Failed to destroy TC hook");
			n--;
		}
	} else if (module_is_xdp) {
		int flags =
		    (attach_point &
		     ~(BPF_TC_INGRESS | BPF_TC_EGRESS)); // Extract XDP flags
		if (bpf_xdp_detach(ifindex, flags, NULL) < 0) {
			print(ERROR, "Failed to detach XDP program");
			return -1;
		}
		n = 1;
	} else {
		print(ERROR, "Unknown eBPF program type");
		return -1;
	}

	return n;
}

int unmount_bpf_module(struct bpf_object *obj, const char *interface_name)
{
	int ret = 0;
	if (detach_bpf_program(obj, interface_name) <= 0) {
		print(ERROR, "Failed to detach eBPF program");
		ret = -1;
	}

	stats_map_fd = -1;
	config_map_fd = -1;

	if (obj)
		bpf_object__close(obj);

	bpf_loaded_program_obj = NULL;
	return ret;
}

int cleanup(void)
{
	if (bpf_loaded_program_obj)
		return unmount_bpf_module(bpf_loaded_program_obj, interface);

	if (stats_map_fd >= 0) {
		close(stats_map_fd);
		stats_map_fd = -1;
	}

	return 0;
}
