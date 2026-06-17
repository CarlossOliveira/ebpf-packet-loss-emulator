<img src="docs/assets/images/ebpf-packet-loss-emulator.jpg" alt="Packet Loss Emulator" />

# eBPF Packet Loss Emulator

[![CI](https://github.com/CarlossOliveira/ebpf-packet-loss-emulator/actions/workflows/CI.yml/badge.svg)](https://github.com/CarlossOliveira/ebpf-packet-loss-emulator/actions/workflows/CI.yml)
[![CodeQL](https://github.com/CarlossOliveira/ebpf-packet-loss-emulator/actions/workflows/github-code-scanning/codeql/badge.svg)](https://github.com/CarlossOliveira/ebpf-packet-loss-emulator/actions/workflows/github-code-scanning/codeql)

---

## Project Overview

**ebpf-packet-loss-emulator** is an extensible network impairment framework built upon the Extended Berkeley Packet Filter (eBPF) technology. The project has been developed to emulate packet loss under controlled and configurable conditions, enabling the evaluation, validation, and performance analysis of applications and network protocols operating in adverse communication environments.

Traditional packet loss emulation mechanisms often rely on user-space packet interception or traffic shaping utilities, which may introduce additional latency, context-switching overhead, and limited visibility into low-level packet processing operations. By leveraging eBPF, packet loss decisions can be performed directly within the Linux kernel networking stack, allowing packet processing to occur closer to the actual data path while maintaining low overhead and high flexibility.

The framework consists of a custom user-space loader implemented in C and a collection of dynamically loadable eBPF modules. The loader is responsible for discovering, loading, configuring, and attaching eBPF programs to user-selected hook points within the Linux networking subsystem. Once attached, the selected eBPF module operates entirely within kernel space and applies packet loss according to the implemented impairment model and runtime configuration parameters.

The current implementation provides support for both Traffic Control (TC) and eXpress Data Path (XDP) attachment mechanisms, enabling packet loss emulation at different stages of packet processing. This allows users to evaluate how attachment location influences performance characteristics, packet visibility, and overall system behaviour.

Beyond packet loss emulation, the framework has been designed with extensibility as a primary architectural objective. New impairment models, traffic manipulation algorithms, and packet processing strategies can be incorporated through additional eBPF modules without requiring modifications to the core loader infrastructure.

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

## Features

- Packet loss emulation directly within the Linux kernel using eBPF;
- Support for both Traffic Control (TC) and eXpress Data Path (XDP) execution contexts;
- Dynamic loading and unloading of eBPF modules at runtime;
- Runtime module configuration through BPF maps;
- Automatic module discovery and management;
- Support for multiple attachment points and execution modes;
- Runtime statistics collection and monitoring;
- Interactive command-line management interface;
- Extensible plug-in architecture for custom impairment models;
- Automated build, testing, and documentation workflows.

---

## Documentation

The project documentation is organised into several documents covering installation, usage, architecture, and framework extensibility.

| Document                | Description                                                                    |
| ----------------------- | ------------------------------------------------------------------------------ |
| `installation.md`       | Installation, dependency management, and build instructions.                   |
| `usage.md`              | Command-line interface, module loading, configuration, and runtime management. |
| `architecture.md`       | Framework architecture, loader design, and internal components.                |
| `networking-stack.md`   | Linux networking stack overview and supported attachment points.               |
| `available-modules.md`  | Description of the currently implemented packet loss modules.                  |
| `module-development.md` | Guidelines for developing and integrating new eBPF modules.                    |
| `conclusion.md`         | Project summary, outcomes, and future work.                                    |

### API Reference

The complete API reference is available [here](/api/index.html).

<p>
  <a href="docs/user-guides/getting-started/setup.md">
    <img src="docs/assets/images/buttons/test.svg"  align="right">
  </a>
</p>
