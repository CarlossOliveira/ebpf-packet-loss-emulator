# Module Development

## Overview

One of the primary design goals of the framework is extensibility.

The architecture has been designed to allow new packet impairment models to be integrated with minimal effort and without requiring modifications to the existing loader implementation.

To achieve this objective, all eBPF modules follow a common structure and expose a standard interface that can be automatically discovered and managed by the framework.

As long as the required conventions are respected, new modules can be compiled, loaded, configured, and monitored through the existing infrastructure without additional integration work.

---

## Module Structure

Every module must provide a valid eBPF program entry point and expose any runtime configuration parameters through the framework-defined metadata sections.

A minimal module implementation is shown below:

```c
#include "globals.h"

char CONFIG_KEYS[][CONFIG_KEY_SIZE] SEC(".config_keys") = {
    {0},
};

SEC("classifier")
int packet_handler(...)
{
    (...)
}
```

The framework relies on this structure to identify:

- Program entry points;
- Runtime configuration keys;
- Shared BPF maps;
- Attachment requirements.

During loading, the ELF parser inspects these sections and automatically registers the module with the framework.

---

## Runtime Configuration

Modules may expose configurable runtime parameters through the `.config_keys` section.

For example:

```c
char CONFIG_KEYS[][CONFIG_KEY_SIZE] SEC(".config_keys") = {
    "packet_loss_probability",
    "seed",
    "burst_length",
    {0},
};
```

Each entry represents a configuration parameter that becomes available through the framework's CLI.

When a module is loaded, the loader automatically parses the configuration section and registers the available parameters.

Values can subsequently be modified at runtime using the `config` command without requiring recompilation, unloading, or reattachment.

This mechanism enables module-specific behaviour to be adjusted dynamically while maintaining a consistent management interface across all implementations.

---

## Shared BPF Maps

To ensure interoperability between user-space and kernel-space components, all modules are expected to use the framework's shared maps.

These maps provide a standardised mechanism for configuration management and statistics collection.

### Statistics Map

```c
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 3);
    __type(key, __u8);
    __type(value, __u64);
} stats_map SEC(".maps");
```

The statistics map is used to expose runtime metrics to the framework.

Typical metrics include:

- Processed packets;
- Dropped packets;
- Execution counters;
- Module-specific performance statistics.

Statistics collected through this map become available through the `stats` command.

---

### Configuration Map

```c
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, MAX_CONFIG_ENTRIES);
    __type(key, char[CONFIG_KEY_SIZE]);
    __type(value, __u64);
} config_map SEC(".maps");
```

The configuration map provides a communication channel between user-space management components and kernel-space packet processing logic.

Configuration values modified through the CLI are stored in this map and can be retrieved directly by the eBPF program during execution.

This mechanism enables runtime behaviour to be updated without interrupting packet processing.

---

## Compilation and Discovery

Any source file following the `*.bpf.c` naming convention and placed within the module directory is automatically compiled into an ELF object during the build process.

```text
src/
└── kernel-space/
    └── bpf/
        └── modules/
```

After compilation, the resulting object file is placed in:

```text
build/kernel-space/bpf/modules/
```

The loader automatically scans this directory during runtime and exposes all discovered modules through the `list` command.

No manual registration or loader modifications are required.

---

## Developing New Packet Impairment Models

The framework is intentionally designed to support a wide range of network impairment techniques beyond simple packet loss emulation.

Potential module implementations include:

- Bernoulli packet loss models;
- Burst packet loss models;
- Delay injection;
- Jitter simulation;
- Bandwidth throttling;
- Packet duplication;
- Packet reordering;
- Queueing and scheduling experiments;
- Protocol-aware impairment models.

Because all modules share a common interface and management infrastructure, new impairment strategies can be introduced without modifying the framework's control plane.

---

## Development Guidelines

When implementing new modules, the following recommendations should be followed:

- Reuse the framework's shared maps whenever possible.
- Expose configurable parameters through the `.config_keys` section.
- Keep packet processing logic isolated within the eBPF program.
- Avoid introducing module-specific assumptions into the loader.
- Ensure compatibility with the verifier across supported kernel versions.
- Maintain consistent statistics reporting through the shared statistics map.

Following these guidelines ensures that new modules remain compatible with the framework's automatic discovery, configuration, and monitoring capabilities.

---

## Design Philosophy

The framework treats packet impairment modules as independent plug-ins.

The loader is intentionally unaware of the internal implementation details of individual modules and instead relies on ELF metadata, shared maps, and standardised interfaces.

This design enables the framework to evolve beyond packet loss emulation and serve as a reusable platform for network experimentation, performance evaluation, and advanced traffic engineering research.

<div style="display:flex; justify-content:space-between;">
<a href="../architecture/modules.md"
   style="
      display:inline-block;
      padding:6px 16px;
      border:1px solid #dadde1;
      border-radius:8px;
      text-decoration:none;
      line-height:1.4;
   ">

   <div style="font-size:0.75rem; color:#6b7280;">
      Previous
   </div>
   <div style="font-weight:600;">
      Modules Implemented
   </div>
</a>

<a href="../../conclusions.md"
   style="
      display:inline-block;
      padding:6px 16px;
      border:1px solid #dadde1;
      border-radius:8px;
      text-decoration:none;
      text-align:right;
      line-height:1.4;
   ">

   <div style="font-size:0.75rem; color:#6b7280;">
      Next
   </div>
   <div style="font-weight:600;">
      Conclusions
   </div>
</a>
</div>
