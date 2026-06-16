# Setup

## Prerequisites

Before installing the framework, ensure that your system satisfies the following requirements:

- Linux operating system with eBPF support.
- A kernel version compatible with the required eBPF features.
- Sufficient privileges to install development packages and load eBPF programs.

> [!WARNING]
> eBPF relies on kernel-specific functionality and may not be fully supported on all Linux distributions or kernel versions. Verify that your environment provides the required eBPF capabilities before proceeding.

---

## Clone the Repository

```bash
git clone https://github.com/CarlossOliveira/ebpf-packet-loss-emulator.git
cd ebpf-packet-loss-emulator
```

---

## Install Dependencies

The project provides an automated dependency installation script that installs the toolchain and libraries required for eBPF development, compilation, loading, and execution.

Using the installation script:

```bash
sudo ./scripts/install_dependencies.sh install
```

Alternatively, dependencies can be installed through the project's Makefile:

```bash
sudo make install
```

---

## Build the Project

Compile the framework and all eBPF modules:

```bash
make all
```

Compiled binaries and object files will be generated in the `build/` directory.

<p>
  <a href="../../README.md">
    <img src="../../assets/images/buttons/prev_home.svg" align="left">
  </a>
  <a href="usage.md">
    <img src="../../assets/images/buttons/next_usage.svg" align="right">
  </a>
</p>
