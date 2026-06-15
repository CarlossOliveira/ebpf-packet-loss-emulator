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

---

# Uninstallation

## Remove Dependencies

To remove all packages installed by the dependency manager:

Using the installation script:

```bash
sudo ./scripts/install_dependencies.sh uninstall
```

Alternatively:

```bash
sudo make uninstall
```

---

## Remove Build Artifacts

Delete all generated binaries, object files, and intermediate build artifacts:

```bash
make clean
```

This operation restores the source tree to a clean state without modifying the project source files.

<div style="display:flex; justify-content:space-between;">
<a href="../../../README.md"
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
      ← Home
   </div>
</a>

<a href="usage.md"
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
      Usage →
   </div>
</a>
</div>
