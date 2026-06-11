#include "bpf_manager.h"

#include "globals.h"

#include "elf_utils.h"
#include "io_utils.h"

#include <errno.h>

static int attach_bpf_program(app_context_t *ctx);
static int detach_bpf_program(app_context_t *ctx);

int open_map(struct bpf_object *obj, char *map_name)
{
	struct bpf_map *map = bpf_object__find_map_by_name(obj, map_name);
	if (!map) {
		print(ERROR, "Failed to find map '%s' in eBPF object", map_name);
		return -1;
	}

	return bpf_map__fd(map);
}

/* LOAD BPF MODULE */

struct bpf_object *load_bpf_module(app_context_t *ctx)
{
	char path[512];
	if (strstr(ctx->bpf.module_name, ".bpf.o"))
		snprintf(path, sizeof(path), "%s/%s", BPF_OBJECT_DIR, ctx->bpf.module_name);
	else
		snprintf(path, sizeof(path), "%s/%s.bpf.o", BPF_OBJECT_DIR, ctx->bpf.module_name);

	ctx->bpf.object = bpf_object__open_file(path, NULL);
	int err = (int)libbpf_get_error(ctx->bpf.object);

	if (err) {
		print(ERROR, "Failed to open eBPF object file: %s", path);
		return NULL;
	}

	err = bpf_object__load(ctx->bpf.object);
	if (err) {
		errno = err < 0 ? -err : err;
		print(ERROR, "Failed to load eBPF object file: %s", path);
		bpf_object__close(ctx->bpf.object);
		return NULL;
	}

	int new_stats_map_fd = open_map(ctx->bpf.object, "stats_map");
	if (new_stats_map_fd < 0) {
		print(ERROR, "Failed to open stats map");
		bpf_object__close(ctx->bpf.object);
		return NULL;
	}

	int new_config_map_fd = open_map(ctx->bpf.object, "config_map");
	if (new_config_map_fd < 0) {
		print(ERROR, "Failed to open config map");
		bpf_object__close(ctx->bpf.object);
		return NULL;
	}

	if (attach_bpf_program(ctx) <= 0) {
		print(ERROR, "Failed to attach eBPF program");
		bpf_object__close(ctx->bpf.object);
		return NULL;
	}

	ctx->bpf.maps.stats_map_fd = new_stats_map_fd;
	ctx->bpf.maps.config_map_fd = new_config_map_fd;

	return ctx->bpf.object;
}

int unload_bpf_module(app_context_t *ctx)
{
	int ret = 0;
	if (detach_bpf_program(ctx) <= 0) {
		print(ERROR, "Failed to detach eBPF program");
		ret = -1;
	}

	ctx->bpf.maps.stats_map_fd = -1;
	ctx->bpf.maps.config_map_fd = -1;

	if (ctx->bpf.object)
		bpf_object__close(ctx->bpf.object);

	ctx->bpf.object = NULL;
	return ret;
}

static int attach_bpf_program(app_context_t *ctx)
{
	if (!ctx->bpf.object) {
		print(ERROR, "No eBPF object loaded");
		return -1;
	}

	ctx->interface.ifindex = if_nametoindex(ctx->interface.name);
	if (ctx->interface.ifindex == 0) {
		print(ERROR, "Invalid interface: %s", ctx->interface.name);
		return -1;
	}

	// Determine program type by checking for known sections in the ELF file
	char bpf_elf_filename[512];
	snprintf(bpf_elf_filename, sizeof(bpf_elf_filename), "%s/%s.bpf.o", BPF_OBJECT_DIR, ctx->bpf.module_name);
	int elf_fd = open_elf_file(bpf_elf_filename);
	if (elf_fd < 0) {
		print(ERROR, "Failed to open eBPF object file for inspection: %s", bpf_elf_filename);
		return -1;
	}

	bool module_is_tc = elf_has_section(elf_fd, "classifier");
	bool module_is_xdp = elf_has_section(elf_fd, "xdp");
	close_elf_file(elf_fd);

	// Load the program fd
	struct bpf_program *prog = bpf_object__find_program_by_name(ctx->bpf.object, "packet_handler");
	if (!prog) {
		print(ERROR, "Failed to find eBPF program");
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

		if (ctx->interface.tc_attach_points & BPF_TC_INGRESS)
			points[n++] = BPF_TC_INGRESS;

		if (ctx->interface.tc_attach_points & BPF_TC_EGRESS)
			points[n++] = BPF_TC_EGRESS;

		if (n == 0) {
			print(ERROR, "No valid attachment point specified for TC module");
			return -1;
		}
		for (int i = 0; i < n; i++) {
			struct bpf_tc_hook hook = {
			    .sz = sizeof(hook),
			    .ifindex = (int)ctx->interface.ifindex,
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
		if (bpf_xdp_attach((int)ctx->interface.ifindex, prog_fd, ctx->interface.xdp_attach_point, NULL) < 0) {
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

static int detach_bpf_program(app_context_t *ctx)
{
	if (ctx->interface.ifindex == 0) {
		print(ERROR, "Invalid interface: %s", ctx->interface.name);
		return -1;
	}

	// Determine program type by checking for known sections in the ELF file
	char bpf_elf_filename[512];
	snprintf(bpf_elf_filename, sizeof(bpf_elf_filename), "%s/%s.bpf.o", BPF_OBJECT_DIR, ctx->bpf.module_name);
	int elf_fd = open_elf_file(bpf_elf_filename);
	if (elf_fd < 0) {
		print(ERROR, "Failed to open eBPF object file for inspection: %s", bpf_elf_filename);
		return -1;
	}

	bool module_is_tc = elf_has_section(elf_fd, "classifier");
	bool module_is_xdp = elf_has_section(elf_fd, "xdp");
	close_elf_file(elf_fd);

	// Load the program fd
	struct bpf_program *prog = bpf_object__find_program_by_name(ctx->bpf.object, "packet_handler");
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
		if (ctx->interface.tc_attach_points & BPF_TC_INGRESS)
			points[n++] = BPF_TC_INGRESS;

		if (ctx->interface.tc_attach_points & BPF_TC_EGRESS)
			points[n++] = BPF_TC_EGRESS;

		if (n == 0) {
			print(ERROR, "No valid attachment point specified for TC module");
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
			    .ifindex = (int)ctx->interface.ifindex,
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
		    .ifindex = (int)ctx->interface.ifindex,
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
		if (bpf_xdp_detach((int)ctx->interface.ifindex, ctx->interface.xdp_attach_point, NULL) < 0) {
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
