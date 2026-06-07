<img src="docs/images/ebpf-packet-loss-emulator.jpg" alt="Packet Loss Emulator">

# eBPF Packet Loss Emulator

[![CI](https://github.com/CarlossOliveira/ebpf-packet-loss-emulator/actions/workflows/CI.yml/badge.svg)](https://github.com/CarlossOliveira/ebpf-packet-loss-emulator/actions/workflows/CI.yml)

## Table of Contents

- [Project Overview](#project-overview)
- [Installation](#installation)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Project Context](#project-context)
  - [What is eBPF?](#what-is-ebpf)
  - [Why eBPF?](#why-ebpf)
  - [ELF Format and Program Loading](#elf-format-and-program-loading)
  - [Linux Networking Stack and Attachment Points](#linux-networking-stack-and-attachment-points)

- [Implementation](#implementation)
  - [Core Components](#core-components)
  - [eBPF Modules](#ebpf-modules)
  - [Hook Points](#hook-points)
  - [Extensibility](#extensibility)

- [Project Objectives](#project-objectives)
- [Sources](#sources)
- [License](#license)

---

## Project Overview

**bpf-packet-loss-emulator** is an extensible network impairment framework built upon the Extended Berkeley Packet Filter (eBPF) technology. The project has been developed to emulate packet loss under controlled and configurable conditions, enabling the evaluation, validation, and performance analysis of applications and network protocols operating in adverse communication environments.

Traditional packet loss emulation mechanisms often rely on user-space packet interception or traffic shaping utilities, which may introduce additional latency, context-switching overhead, and limited visibility into low-level packet processing operations. By leveraging eBPF, packet loss decisions can be performed directly within the Linux kernel networking stack, allowing packet processing to occur closer to the actual data path while maintaining low overhead and high flexibility.

The framework consists of a custom user-space loader implemented in C and a collection of dynamically loadable eBPF modules. The loader is responsible for discovering, loading, configuring, and attaching eBPF programs to user-selected hook points within the Linux networking subsystem. Once attached, the selected eBPF module operates entirely within kernel space and applies packet loss according to the implemented impairment model and runtime configuration parameters.

The current implementation provides support for both Traffic Control (TC) and eXpress Data Path (XDP) attachment mechanisms, enabling packet loss emulation at different stages of packet processing. This allows users to evaluate how attachment location influences performance characteristics, packet visibility, and overall system behaviour.

Beyond packet loss emulation, the framework has been designed with extensibility as a primary architectural objective. New impairment models, traffic manipulation algorithms, and packet processing strategies can be incorporated through additional eBPF modules without requiring modifications to the core loader infrastructure.

---

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/CarlossOliveira/ebpf-packet-loss-emulator.git
```

### 2. Navigate to the Project Directory

```bash
cd ebpf-packet-loss-emulator
```

### 3. Install Dependencies

```bash
sudo ./scripts/install_dependencies.sh install
```

### 4. Remove Dependencies

```bash
sudo ./scripts/install_dependencies.sh uninstall
```

The installation script automatically installs the toolchain and libraries required for eBPF development, compilation, loading, and execution.

**NOTE:** It's important to notice that eBPF development requires a compatible Linux Kernel and due to its reliance on specific kernel features, it may not be supported on all distributions or versions. Ensure that your system meets the necessary requirements for eBPF development before proceeding with installation.

---

## Usage

The framework exposes an interactive command-line interface (CLI) that allows users to load eBPF modules, modify runtime parameters, inspect execution statistics, and dynamically switch between packet loss implementations.

To launch the emulator:

```bash
sudo ./packet_loss_emulator <network_interface> [flags]
```

where:

- `<network_interface>` specifies the target network interface;
- `[flags]` determines the attachment mechanism and execution context used by the eBPF program.

### Available Flags

| Flag                             | Description                                                    |
| -------------------------------- | -------------------------------------------------------------- |
| `egress` / `BPF_TC_EGRESS`       | Attaches the eBPF program to the Traffic Control egress hook.  |
| `ingress` / `BPF_TC_INGRESS`     | Attaches the eBPF program to the Traffic Control ingress hook. |
| `driver` / `XDP_FLAGS_DRV_MODE`  | Attaches the eBPF program using XDP Native mode.               |
| `generic` / `XDP_FLAGS_SKB_MODE` | Attaches the eBPF program using XDP Generic mode.              |
| `offload` / `XDP_FLAGS_HW_MODE`  | Attaches the eBPF program using XDP Hardware Offload mode.     |

To choose more than one flag, simply concatenate them with a pipe (`|`):

Example:

```bash
sudo ./packet_loss_emulator eth0 'egress|ingress|driver'
```

**NOTE:** The `offload` flag requires compatible hardware and may not be supported on all systems.
**NOTE:** If no flags are provided, the default attachment points are TC Ingress and XDP Generic.
**NOTE:** You can only select one XDP execution mode at a time, but you can combine XDP with TC to compare different attachment points.

### Runtime Module Selection

Upon startup, the loader scans the available eBPF modules and presents them to the user for selection.

Example:

```bash
Press Ctrl+C to exit, Ctrl+\ to change module.

Available eBPF modules:
-> tc_bernoulli
-> xdp_bernoulli

Enter the name of the eBPF module to load:
```

After a module is selected, the loader performs the following operations:

1. Loads the corresponding ELF object file;
2. Creates and initializes all required BPF maps;
3. Loads the eBPF bytecode into the kernel;
4. Attaches the program to the specified hook point;
5. Starts the interactive runtime environment.

The selected module immediately begins processing packets according to its internal logic and configuration state.

### Interactive CLI

Once loaded, the framework exposes a runtime management interface:

```bash
Available commands:

- config : Send module-specific configuration parameters.
- stats  : Display packet processing statistics.
- change : Switch to a different eBPF module.
- clear  : Clear the terminal window.
- help   : Display command documentation.
- exit   : Terminate the emulator.
```

Configuration parameters are transmitted to kernel-space through BPF maps, allowing packet loss behaviour to be modified dynamically without unloading or recompiling the eBPF program.

The framework additionally supports signal-based control:

| Signal            | Action                                |
| ----------------- | ------------------------------------- |
| SIGINT (Ctrl+C)   | Gracefully terminate the emulator     |
| SIGTERM           | Gracefully terminate the emulator     |
| SIGQUIT (Ctrl+\\) | Trigger the module-switching workflow |

---

## Project Structure

```text
ebpf-packet-loss-emulator
├── build
│   ├── bin
│   │   └── bpf-packet-loss-emulator
│   └── bpf
│       └── modules
│           ├── tc_bernoulli.bpf.o
│           └── xdp_bernoulli.bpf.o
├── documentation
│   ├── egress_data_path.jpg
│   ├── ingress_data_path.jpg
│   └── legend.jpg
├── LICENSE
├── Makefile
├── README.md
├── scripts
│   └── dependencie_manager.sh
└── src
    ├── bpf
    │   ├── helpers.bpf.h
    │   ├── modules
    │   │   ├── tc_bernoulli.bpf.c
    │   │   └── xdp_bernoulli.bpf.c
    │   └── utils
    │       └── math_utils.bpf.h
    ├── core
    │   ├── cleanup.c
    │   ├── globals.c
    │   ├── helpers.c
    │   ├── setup.c
    │   └── utils
    │       ├── elf_utils.c
    │       ├── io_utils.c
    │       ├── privilege_utils.c
    │       ├── string_utils.c
    │       └── time_utils.c
    ├── include
    │   ├── globals.h
    │   ├── utils
    │   │   ├── elf_utils.h
    │   │   ├── io_utils.h
    │   │   ├── privilege_utils.h
    │   │   ├── string_utils.h
    │   │   └── time_utils.h
    │   └── vmlinux.h
    └── main.c
```

The project is organised into several directories that separate core functionality, eBPF modules, utilities, and documentation. The `src` directory contains all source code, while the `build` directory holds compiled binaries and object files. The `documentation` directory includes diagrams illustrating the Linux networking stack and attachment points. The `scripts` directory contains the dependency management script, and the project root includes the Makefile, license, and README documentation. To integrate new eBPF modules, simply add a new `.bpf.c` file to the `src/bpf/modules/` directory following the required structure and interface conventions. All modules placed in this directory will be automatically discovered and made available for selection at runtime after compilation.

---

## Project Context

Understanding the design decisions behind the framework requires familiarity with several core technologies and concepts that underpin modern Linux packet processing. This section introduces the fundamental concepts required to understand the implementation presented throughout the remainder of this document.

---

### What is eBPF?

Extended Berkeley Packet Filter (eBPF) is a programmable execution environment embedded within the Linux kernel that enables the safe execution of user-defined code without requiring custom kernel modules.

Originally derived from the Berkeley Packet Filter (BPF) packet filtering mechanism, eBPF has evolved into a general-purpose kernel instrumentation and packet processing framework capable of extending kernel functionality across multiple subsystems.

eBPF programs are executed inside a sandboxed virtual machine embedded within the kernel. Prior to execution, each program undergoes rigorous static verification by the kernel verifier to ensure:

- Memory safety;
- Bounded execution;
- Type correctness;
- Safe pointer usage;
- Absence of unsafe kernel interactions.

Programs that fail verification are rejected before deployment, preventing crashes, deadlocks, and other forms of kernel instability.

Today, eBPF is widely used across a variety of domains, including:

- High-performance packet processing;
- Traffic engineering and filtering;
- System observability;
- Performance profiling;
- Security monitoring;
- Runtime tracing;
- Container networking;
- Service mesh implementations.

Because eBPF programs execute directly within the kernel execution path, they can observe and manipulate packets with significantly lower overhead than traditional user-space solutions.

---

### Why eBPF?

The decision to employ eBPF as the foundation of the packet loss emulator was motivated by several architectural and performance-related advantages.

#### Performance

eBPF programs execute directly within the packet processing path, eliminating the need for expensive user-space context switches.

Furthermore, certain execution modes, such as XDP Hardware Offload, allow packet processing logic to be executed directly on compatible Network Interface Controllers (NICs), reducing CPU utilisation and latency even further.

#### Flexibility

eBPF programs can be loaded, unloaded, and replaced dynamically at runtime.

This capability allows packet impairment models to be modified without requiring kernel recompilation, application restarts, or system reboots, making eBPF particularly suitable for experimentation and testing environments.

#### Safety

All programs must pass kernel verification before execution.

The verifier guarantees that deployed programs cannot perform unsafe memory operations, execute unbounded loops, or compromise kernel stability.

This provides a level of safety traditionally unavailable when extending kernel functionality.

#### Observability

eBPF provides direct access to kernel-level telemetry and execution statistics.

This capability enables the collection of detailed runtime metrics and packet processing information that would otherwise be inaccessible through conventional user-space tooling.

---

### ELF Format and Program Loading

eBPF programs are commonly compiled into ELF (Executable and Linkable Format) object files.

An ELF object contains not only the eBPF bytecode itself, but also metadata describing:

- Program sections;
- BPF maps;
- Relocation information;
- Configuration structures;
- License declarations;
- Program entry points.

The user-space loader implemented by this project is responsible for parsing these ELF objects, extracting the required sections, creating the corresponding kernel resources, and loading the resulting programs into the kernel.

This architecture provides a clear separation between:

- User-space orchestration logic;
- Kernel-space packet processing logic.

As a consequence, new eBPF modules can be developed independently from the loader, significantly simplifying extensibility and long-term maintenance.

---

### Linux Networking Stack and Attachment Points

The performance characteristics, packet visibility, and execution context available to an eBPF program are largely determined by the location at which it is attached within the Linux networking subsystem. Consequently, understanding the packet traversal path through the networking stack is essential when evaluating the behaviour of packet processing applications and network impairment mechanisms.

The Linux networking architecture consists of a combination of hardware and software components responsible for transmitting and receiving packets between applications and physical network interfaces. These components include user-space applications, kernel networking subsystems, device drivers, memory management structures, and network interface controllers (NICs).

The framework supports both eXpress Data Path (XDP) and Traffic Control (TC) attachment mechanisms, enabling packet loss emulation at multiple stages of packet processing. Each attachment point provides a distinct trade-off between execution performance, available packet context, and hardware compatibility.

#### Legend

The following legend is used throughout the networking stack diagrams.

#### Legend

The following symbols are used throughout the networking stack diagrams to represent the various hardware and software components involved in packet processing.

| Symbol                                        | Component                          | Description                                                                                             |
| --------------------------------------------- | ---------------------------------- | ------------------------------------------------------------------------------------------------------- |
| <img src="docs/images/linux.png" width="40">  | Linux Kernel                       | Components and subsystems executing within kernel space.                                                |
| <img src="docs/images/user.png" width="40">   | User Space                         | User applications and processes interacting with the networking stack.                                  |
| <img src="docs/images/RAM.png" width="40">    | System Memory (RAM)                | Main memory used to store packet buffers, SKBs, descriptors, and runtime data structures.               |
| <img src="docs/images/CPU.png" width="40">    | CPU                                | Processing units responsible for executing networking and application workloads.                        |
| <img src="docs/images/NIC.png" width="40">    | Network Interface Controller (NIC) | Hardware device responsible for transmitting and receiving network traffic.                             |
| <img src="docs/images/driver.png" width="40"> | Network Driver                     | Software component providing the interface between the Linux networking subsystem and the NIC hardware. |

#### Egress Data Path

![Linux Networking Stack - Egress Path](docs/images/egress_data_path.jpg)

The egress path describes the sequence of operations performed when an application transmits data through a network interface.

Transmission begins in user space when an application invokes a socket transmission primitive, such as `send()`. The Linux networking subsystem subsequently transforms the application data into a network packet and prepares it for hardware transmission.

The principal processing stages are:

1. User-space transmission request (`send()`).
2. Creation and initialization of a Socket Buffer (SKB).
3. Ethernet frame generation.
4. Optional VLAN encapsulation.
5. Generic Segmentation Offload (GSO) or TCP Segmentation Offload (TSO).
6. Network driver transmission preparation.
7. DMA descriptor creation and transmit ring buffer insertion.
8. Direct Memory Access (DMA) transfer to the NIC.
9. Hardware transmission through the MAC and PHY layers.

Within this path, the TC egress attachment point is located immediately before the packet is handed to the network driver for transmission. At this stage, the packet has already been fully constructed and contains complete protocol metadata, routing information, and networking subsystem state.

This execution context makes TC egress particularly suitable for traffic shaping, filtering, scheduling, and packet loss emulation scenarios where access to higher-layer protocol information is required.

##### Socket Buffers (SKBs)

The Socket Buffer (`sk_buff`) is the fundamental packet representation used by the Linux networking subsystem. It stores packet payloads, protocol headers, routing metadata, timestamps, and various internal networking structures.

Many kernel networking subsystems, including Traffic Control, Netfilter, and routing components, operate directly on SKB structures.

##### Segmentation Offloading

Modern Linux systems employ several offloading mechanisms to reduce CPU overhead during packet transmission.

**Generic Segmentation Offload (GSO)** allows large packets to remain aggregated within the kernel until later stages of processing.

**TCP Segmentation Offload (TSO)** extends this concept by delegating packet segmentation directly to compatible network hardware.

These optimisations reduce CPU utilisation and improve transmission throughput, particularly under high traffic loads.

##### Direct Memory Access (DMA)

Direct Memory Access enables network interface controllers to access packet buffers directly from system memory without continuous CPU intervention.

By bypassing intermediate copy operations, DMA significantly reduces packet transmission overhead and improves overall network performance.

---

#### Ingress Data Path

![Linux Networking Stack - Ingress Path](docs/images/ingress_data_path.jpg)

The ingress path describes the sequence of operations performed when packets arrive from the network and are delivered to applications.

Packet reception begins at the physical network interface and progresses through multiple hardware and software layers before reaching user space.

The principal processing stages are:

1. Physical frame reception through the PHY layer.
2. Processing by the Media Access Control (MAC) controller.
3. Hardware packet parsing.
4. DMA transfer into host memory.
5. Placement into receive ring buffers.
6. Interrupt generation through MSI-X.
7. Driver polling using NAPI.
8. Generic Receive Offload (GRO).
9. SKB allocation and construction.
10. Ethernet and VLAN processing.
11. Delivery to the application through `recv()`.

Unlike the egress path, the ingress path provides multiple locations where packet processing can be intercepted before the packet reaches the upper networking layers.

---

##### NAPI and Interrupt Mitigation

Modern Linux network drivers utilise the New API (NAPI) framework to improve packet reception efficiency.

Instead of generating an interrupt for every received packet, NAPI combines interrupt-driven and polling-based processing. Upon packet arrival, the NIC generates an interrupt that schedules a polling cycle. Subsequent packets are then processed through polling until the receive queue is drained.

This approach significantly reduces interrupt overhead and improves scalability under high packet rates.

##### Generic Receive Offload (GRO)

Generic Receive Offload performs the inverse operation of GSO.

Rather than processing every packet individually, GRO aggregates compatible packets into larger buffers before protocol processing occurs.

This reduces per-packet overhead and improves overall receive-side performance.

---

#### XDP Attachment Points

eXpress Data Path (XDP) is a high-performance packet processing framework designed to execute eBPF programs as early as possible in the packet reception path.

By executing before most of the networking stack is traversed, XDP minimises packet processing latency and maximises throughput.

The framework supports all three XDP execution modes.

##### XDP Generic Mode

XDP Generic mode executes after SKB allocation within the conventional networking stack.

Because it does not require driver-specific support, it offers the broadest compatibility across network interfaces and kernel versions. However, it also incurs the highest processing overhead among XDP execution modes due to the cost of SKB construction.

##### XDP Native Mode

XDP Native mode executes directly within the network driver's receive path before SKB allocation occurs.

By avoiding a substantial portion of the networking stack, this mode achieves significantly lower latency and higher throughput than Generic mode.

Native mode is typically considered the preferred software-based XDP execution model whenever hardware support is available.

##### XDP Hardware Offload Mode

XDP Hardware Offload extends execution beyond the operating system itself by offloading eBPF programs directly onto supported network interface controllers.

In this configuration, packets may be processed before they are transferred to host memory, resulting in extremely low latency and minimal CPU utilisation.

The primary limitation of this mode is the reduced packet context available to the program and the dependence on vendor-specific hardware support.

---

#### Traffic Control Attachment Points

Traffic Control (TC) operates at a higher abstraction level than XDP and relies on the SKB infrastructure provided by the Linux networking stack.

Unlike XDP, TC supports packet processing in both ingress and egress directions.

Although TC introduces additional processing overhead, it provides significantly richer packet context and greater integration with the kernel networking subsystem.

##### TC Ingress

TC ingress programs execute after packet reception has completed and the packet has been converted into an SKB.

This attachment point provides access to extensive packet metadata, routing information, and protocol state, making it suitable for advanced packet inspection and manipulation tasks.

##### TC Egress

TC egress programs execute immediately before packets are transmitted through the network driver.

Since packets have already traversed most of the networking stack, this attachment point provides complete visibility into protocol headers and packet metadata.

Consequently, TC egress is particularly well suited for traffic shaping, packet scheduling, traffic engineering, and packet loss emulation.

---

#### Attachment Point Comparison

| Attachment Point | Execution Context   | Packet Visibility | Performance |
| ---------------- | ------------------- | ----------------- | ----------- |
| XDP Offload      | NIC Hardware        | Limited           | Very High   |
| XDP Native       | Driver Receive Path | Limited           | High        |
| XDP Generic      | SKB Layer           | Moderate          | Medium      |
| TC Ingress       | Networking Stack    | Extensive         | Medium      |
| TC Egress        | Networking Stack    | Extensive         | Medium      |

The choice of attachment point ultimately depends on the objectives of the experiment. XDP is generally preferred when maximising throughput and minimising latency are primary concerns, whereas TC offers richer packet context and greater flexibility for packet manipulation and network impairment scenarios.

---

## Implementation

Having established the theoretical foundations required to understand packet processing within the Linux networking subsystem, this section describes the architecture and implementation of the framework.

The project follows a modular architecture composed of a user-space loader and a collection of dynamically loadable eBPF modules. This separation of responsibilities enables independent development of packet processing logic while preserving a stable and reusable loading infrastructure.

The implementation has been designed around three primary objectives:

- Modularity;
- Extensibility;
- Runtime configurability.

Through this architecture, new packet impairment models can be integrated without requiring modifications to the loader, while configuration parameters can be adjusted dynamically during execution through the provided command-line interface.

### Core Components

The framework is composed of a set of loosely coupled components that collectively provide module discovery, program loading, runtime configuration, statistics collection, and packet processing capabilities.

The architecture follows a clear separation between user-space orchestration logic and kernel-space packet processing logic, allowing each layer to evolve independently while maintaining a consistent interface.

#### `main.c`

`main.c` serves as the primary entry point of the application and orchestrates the complete lifecycle of eBPF modules.

Its responsibilities include:

- Parsing command-line arguments;
- Validating user-provided configuration;
- Discovering available eBPF modules;
- Loading and attaching selected programs;
- Initializing BPF maps;
- Managing communication between user space and kernel space;
- Providing the interactive command-line interface;
- Handling runtime signals;
- Collecting and displaying statistics;
- Coordinating module switching operations.

The file effectively acts as the control plane of the framework, while packet processing itself remains entirely within the eBPF programs executing in kernel space.

---

#### `globals.c` and `globals.h`

The files `globals.c` and `globals.h` define the shared infrastructure used throughout both the loader and the eBPF modules.

These files contain:

- Global constants;
- Common data structures;
- BPF map declarations;
- Configuration limits;
- Runtime state definitions;
- Shared utility functions.

Centralising these definitions ensures consistency across all components and significantly simplifies the development of new modules.

---

#### Module Directory

```text
src/
└── bpf/
    └── modules/
```

The `src/bpf/modules/` directory contains all loadable eBPF implementations.

Each module encapsulates a specific packet processing strategy and can be independently compiled, loaded, attached, and configured by the framework.

This approach enables the introduction of new impairment models without requiring modifications to the loader implementation (plugin architecture).

---

#### Loader Architecture

The custom loader forms the core of the framework's runtime environment.

Its primary responsibilities include:

1. Module discovery;
2. ELF parsing;
3. BPF map creation;
4. Program loading;
5. Program attachment;
6. Runtime configuration;
7. Statistics collection.

The loading workflow can be summarised as follows:

```text
User Selection
       │
       ▼
Module Discovery
       │
       ▼
ELF Parsing
       │
       ▼
Map Creation
       │
       ▼
Program Verification
       │
       ▼
Kernel Loading
       │
       ▼
Hook Attachment
       │
       ▼
Runtime Execution
```

This architecture enables complete decoupling between module implementation and loading infrastructure.

As a consequence, newly developed eBPF programs can immediately leverage existing functionality without requiring additional loader modifications.

---

#### Module Discovery

Upon startup, the framework automatically scans the module directory and identifies all available eBPF implementations.

Each discovered module is presented to the user through the interactive selection interface.

This discovery mechanism eliminates the need for hardcoded module registration and ensures that newly added modules are immediately available after compilation.

The design intentionally follows a plug-in style architecture, allowing the framework to scale as additional packet processing models are developed.

---

#### Program Lifecycle

Each eBPF module follows a well-defined lifecycle:

1. Discovery;
2. Selection;
3. ELF parsing;
4. Map creation;
5. Program verification;
6. Kernel loading;
7. Hook attachment;
8. Runtime execution;
9. Statistics collection;
10. Detachment and cleanup.

This lifecycle is managed entirely by the loader, allowing module developers to focus exclusively on packet processing logic.

---

### eBPF Modules

The initial implementation includes two packet loss modules that demonstrate the framework's extensibility while providing support for multiple attachment mechanisms.

#### `tc_bernoulli.bpf.c`

This module implements a Bernoulli packet loss model using Traffic Control attachment points.

The program evaluates each packet independently and applies a probabilistic drop decision according to a configurable loss probability.

The module can be attached to:

- TC Ingress;
- TC Egress.

Because Traffic Control operates on SKB structures, the module has access to extensive packet metadata and networking subsystem context.

---

#### `xdp_bernoulli.bpf.c`

This module implements the same Bernoulli packet loss model using XDP attachment points.

Unlike the TC implementation, packet processing occurs much earlier in the receive path, reducing processing overhead and improving throughput.

The module supports:

- XDP Generic Mode;
- XDP Native Mode;
- XDP Hardware Offload Mode.

This allows direct comparison of packet loss behaviour across multiple execution contexts.

---

#### Bernoulli Packet Loss Model

Both modules implement a Bernoulli-based stochastic packet loss process.

For every packet received by the program, an independent probabilistic decision is performed:

- Packet accepted with probability `1 - p`;
- Packet dropped with probability `p`.

Where:

- `p` represents the configured packet loss probability.

Because each packet is evaluated independently, the resulting packet loss pattern approximates a memoryless random process.

Although relatively simple, this model provides a useful baseline for evaluating application resilience and network protocol behaviour under adverse conditions.

---

#### Runtime Statistics

Each module continuously collects execution statistics that can be queried through the CLI.

The collected metrics include:

- Total processed packets;
- Total dropped packets;
- Total execution time;

These statistics are maintained within kernel-space BPF maps and periodically retrieved by the user-space loader that presents them to the user via the command-line interface and a logging system.

This design minimizes communication overhead while preserving real-time observability.

---

### Hook Points

The framework currently supports both Traffic Control and XDP attachment mechanisms.

From an implementation perspective, attachment is abstracted by the loader, allowing modules to remain largely independent of the underlying hook selection process.

Depending on the selected execution mode, the loader automatically performs the appropriate attachment procedure and manages all required kernel resources.

This abstraction enables module developers to focus on packet processing logic rather than attachment-specific implementation details.

Consequently, a single packet loss model can be implemented across multiple hook points while maintaining a consistent runtime interface.

---

### Extensibility

Extensibility constitutes one of the primary architectural objectives of the framework.

The system has been designed such that new packet impairment models can be integrated with minimal effort and without modifications to the existing loader implementation.

To achieve this objective, all eBPF modules adhere to a common structure and interface.

#### Required Structure

Every module must expose a program entry point that can be identified by the loader and attached to the appropriate hook point.

A simplified template is shown below:

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

The framework relies on this structure to automatically identify configuration sections, program entry points, and required maps.

As long as these conventions are respected, new modules can be compiled and loaded without requiring any modifications to the loader itself.

---

#### Configuration Interface

Each module may expose its own runtime configuration parameters through the `.config_keys` section.

For example:

```c
char CONFIG_KEYS[][CONFIG_KEY_SIZE] SEC(".config_keys") = {
    "packet_loss_probability",
    "seed",
    "burst_length",
    {0},
};
```

During program loading, the framework automatically parses these configuration keys and exposes them through the CLI.

This allows module-specific behaviour to be modified dynamically while the program remains attached and executing.

Consequently, developers can implement sophisticated packet impairment models while retaining a consistent management interface.

---

#### Shared BPF Maps

To ensure interoperability between modules and the loader, several maps are required by all implementations.

##### Statistics Map

```c
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 3);
    __type(key, __u8);
    __type(value, __u64);
} stats_map SEC(".maps");
```

Stores packet processing statistics and execution metrics.

##### Configuration Map

```c
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, MAX_CONFIG_ENTRIES);
    __type(key, char[CONFIG_KEY_SIZE]);
    __type(value, __u64);
} config_map SEC(".maps");
```

Provides a communication channel between user-space configuration commands and kernel-space packet processing logic.

##### Persistent State Map

```c
typedef struct {
    __u8 data[BPF_STATE_SIZE];
} bpf_state_t;

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 1);
    __type(key, char[CONFIG_KEY_SIZE]);
    __type(value, bpf_state_t);
} bpf_module_memory SEC(".maps");
```

Provides persistent kernel-space storage for module-specific state.

This mechanism enables stateful packet processing algorithms without requiring external storage or user-space synchronization.

This map is optional and can be utilised by modules that require state persistence across packet processing iterations.

---

#### Developing New Modules

Any `*.bpf.c` file placed within the module directory and conforming to the required interface can be automatically discovered, loaded, and executed by the framework.

This architecture allows the project to evolve beyond simple packet loss emulation and support more sophisticated network impairment models, including:

- Burst packet loss;
- Delay injection;
- Jitter simulation;
- Bandwidth throttling;
- Packet duplication;
- Packet reordering;
- Protocol-aware impairment models.

As a result, the framework provides a reusable foundation for experimentation, performance evaluation, and advanced network testing research.

---

## Project Objectives

The primary objective of this project was to design and implement a flexible and extensible packet loss emulation framework capable of operating directly within the Linux kernel networking stack through eBPF.

In addition to the implementation of the emulator itself, the project served as a practical exploration of modern Linux networking technologies, kernel extensibility mechanisms, and systems programming methodologies.

The project was developed with the following goals:

- Explore the eBPF ecosystem and its integration with the Linux networking stack;
- Understand the architectural differences between TC and XDP execution contexts;
- Develop a reusable and extensible eBPF loading infrastructure;
- Investigate packet processing behaviour at different stages of the networking pipeline;
- Apply modern software engineering practices to a systems programming project;
- Gain practical experience with ELF loading, kernel-user communication, and BPF maps;
- Develop proficiency in C programming and commonly used user-space tooling;
- Design and maintain automated CI/CD workflows for building and validating the project;
- Create a foundation for future experimentation with advanced network impairment models.

Ultimately, the project aims not only to provide a functional packet loss emulator, but also to serve as a learning platform for understanding the technologies and methodologies involved in contemporary Linux systems development.

---

### Educational and Technical Outcomes

Beyond its primary objective as a packet loss emulation framework, this project served as a practical platform for exploring several aspects of systems programming and software engineering.

The development process provided hands-on experience with:

- eBPF program development and verification;
- Linux kernel networking internals;
- ELF object formats and executable loading mechanisms;
- Traffic Control (TC) and eXpress Data Path (XDP);
- High-performance packet processing techniques;
- Runtime configuration through BPF maps;
- User-space and kernel-space communication mechanisms;
- Continuous Integration and Continuous Deployment (CI/CD) workflows;
- Software modularity and extensibility principles;
- Low-level debugging and observability techniques.

The project also provided an opportunity to strengthen proficiency in modern C development practices, including modular software design, memory management, error handling strategies, build system organisation, and the use of common user-space libraries and utilities.

Throughout the implementation, particular attention was given to maintainability, code readability, separation of concerns, and extensibility, with the goal of producing a framework that could be expanded beyond its original scope while remaining approachable for future development.

---

## Conclusion

This project demonstrates how eBPF can be leveraged to implement flexible and efficient network impairment mechanisms directly within the Linux kernel networking stack.

By combining a modular user-space loader with dynamically attachable eBPF programs, the framework provides a practical environment for studying packet loss behaviour across multiple execution contexts, including Traffic Control and eXpress Data Path.

Beyond its immediate functionality as a packet loss emulator, the project serves as a platform for exploring kernel extensibility, high-performance packet processing, and modern Linux networking technologies. The architecture has been deliberately designed to facilitate future expansion, enabling the integration of increasingly sophisticated impairment models and packet processing strategies.

As eBPF continues to play an increasingly important role in networking, observability, and systems programming, frameworks such as this provide valuable opportunities for experimentation, research, and practical learning within the Linux ecosystem.

---

## Sources

- [eBPF Documentation](https://ebpf.io/)
- [libbpf Documentation](https://libbpf.readthedocs.io/en/latest/index.html)
- [Linux Network Stack Exploration](https://jecklgamis.com/guides/linux-packet-journey?utm_source=chatgpt.com#inbound)

---

## License

This project is licensed under the GPL-3.0 License.

See the [LICENSE](LICENSE) file for additional details.
