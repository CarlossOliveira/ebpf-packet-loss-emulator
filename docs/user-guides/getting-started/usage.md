# Usage

The framework provides an interactive command-line interface (CLI) for loading, configuring, monitoring, and unloading eBPF packet loss emulation modules at runtime.

## Starting the Emulator

Launch the emulator using:

```bash
./ebpf-packet-loss-emulator [OPTIONS]
```

### Available Options

| Option                           | Description                                                                |
| -------------------------------- | -------------------------------------------------------------------------- |
| `-r`, `--run <COMMAND_SEQUENCE>` | Execute a predefined sequence of CLI commands and terminate.               |
| `-d`, `--dump <FILE_PATH>`       | Redirect the application's standard output (STDOUT) to the specified file. |

### Examples

Execute a predefined workflow:

```bash
./ebpf-packet-loss-emulator \
  --run "list; load -m tc_bernoulli -i veth0; exec sleep 15; stats; exit"
```

Execute a workflow and redirect all output to a file:

```bash
./ebpf-packet-loss-emulator \
  --run "list; exit" \
  --dump emulator_state.log
```

When no command sequence is provided, the framework starts in interactive mode.

---

## Available Commands

| Command       | Description                                                 |
| ------------- | ----------------------------------------------------------- |
| `list`        | List all available eBPF modules.                            |
| `load`        | Load and attach an eBPF module.                             |
| `unload`      | Detach and unload the currently active module.              |
| `config`      | Modify module-specific configuration parameters.            |
| `stats`       | Display runtime statistics collected by the active module.  |
| `set-default` | Configure default values used by the `load` command.        |
| `exec`        | Execute a shell command from within the interactive CLI.    |
| `clear`       | Clear the terminal screen.                                  |
| `help`        | Display command documentation and usage information.        |
| `exit`        | Terminate the emulator and release all allocated resources. |

> [!NOTE]
> All commands support the `-h` and `--help` options, which display detailed usage information, available parameters, and practical examples.

---

## Loading eBPF Modules

Modules are loaded dynamically using the `load` command:

```bash
load -m <module_name> -i <interface_name> [-a <attach_point>]
```

### Required Parameters

| Option              | Description                            |
| ------------------- | -------------------------------------- |
| `-m`, `--module`    | Name of the eBPF module to load.       |
| `-i`, `--interface` | Network interface used for attachment. |

> [!NOTE]
> If a default interface has been configured using `set-default interface <name>`, the `--interface` parameter may be omitted. In this case, the configured default interface is used automatically.

### Optional Parameters

| Option                 | Description                                                |
| ---------------------- | ---------------------------------------------------------- |
| `-a`, `--attach-point` | Attachment point(s) where the eBPF program will be loaded. |

---

## Supported Attachment Points

### Traffic Control (TC)

The following TC attachment points may be combined:

| Alias     | Kernel Constant  |
| --------- | ---------------- |
| `ingress` | `BPF_TC_INGRESS` |
| `egress`  | `BPF_TC_EGRESS`  |

### eXpress Data Path (XDP)

Only one XDP execution mode may be selected:

| Alias     | Kernel Constant      |
| --------- | -------------------- |
| `generic` | `XDP_FLAGS_SKB_MODE` |
| `driver`  | `XDP_FLAGS_DRV_MODE` |
| `offload` | `XDP_FLAGS_HW_MODE`  |

Multiple attachment points can be specified using the pipe (`|`) separator.

Examples:

```bash
load -m tc_bernoulli -i veth0 -a ingress|egress
```

```bash
load -m xdp_bernoulli -i eth0 -a driver
```

> [!NOTE]
> TC ingress and egress attachment points may be combined freely.

> [!WARNING]
> XDP execution modes are mutually exclusive. Only one of `generic`, `driver`, or `offload` may be selected.

> [!WARNING]
> Hardware offload support depends on NIC and driver capabilities and may not be available on all systems.

---

## Default Configuration

The framework supports persistent default values for module loading operations.

### Configure a Default Interface

```bash
set-default interface eth0
```

### Configure Default Attachment Points

```bash
set-default attachment-point ingress
```

```bash
set-default attachment-point ingress|egress
```

### Clear All Defaults

```bash
set-default clear
```

When a parameter is omitted from the `load` command, the corresponding default value is used automatically, if configured.

If no default attachment point is defined, the framework applies the following defaults:

| Technology | Default                        |
| ---------- | ------------------------------ |
| TC         | `ingress (BPF_TC_INGRESS)`     |
| XDP        | `generic (XDP_FLAGS_SKB_MODE)` |

---

## Runtime Configuration

Active modules can be reconfigured dynamically without recompilation, unloading, or reattachment.

Example:

```bash
config packet_loss_percentage=100
```

Configuration values are propagated to kernel space through BPF maps, allowing packet-processing behaviour to be modified while the program remains active.

---

## Runtime Statistics

Statistics collected by the active eBPF module can be displayed at any time:

```bash
stats
```

Statistics are retrieved directly from kernel-space BPF maps and presented through the CLI.

---

## Typical Workflow

A typical session may look as follows:

```text
list
load -m tc_bernoulli -i eth0
config packet_loss_percentage=10
stats
unload
exit
```

---

## Module Lifecycle

When a module is loaded, the framework performs the following operations:

1. Load the corresponding ELF object file.
2. Create and initialise the required BPF maps.
3. Verify and load the eBPF bytecode into the kernel.
4. Attach the program to the selected hook point(s).
5. Expose runtime configuration and monitoring capabilities through the CLI.

Once attached, the module immediately begins processing packets according to its implementation and current configuration.

---

## Signal Handling

The framework supports signal-based control mechanisms:

| Signal               | Action                                 |
| -------------------- | -------------------------------------- |
| `SIGINT` (`Ctrl+C`)  | Gracefully terminate the application.  |
| `SIGTERM`            | Gracefully terminate the application.  |
| `SIGQUIT` (`Ctrl+\`) | Trigger the module switching workflow. |

<div style="display:flex; justify-content:space-between;">
<a href="setup.md"
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
      Setup
   </div>
</a>

<a href="cleanup.md"
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
      Cleanup
   </div>
</a>
</div>
