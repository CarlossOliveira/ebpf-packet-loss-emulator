# Available Modules

## Overview

The framework currently provides two packet loss emulation modules implementing a Bernoulli packet loss model across different execution contexts.

Both modules expose a common runtime configuration interface and statistics collection mechanism, allowing their behaviour and performance characteristics to be compared under identical experimental conditions.

---

## `tc_bernoulli.bpf.c`

The `tc_bernoulli` module implements a Bernoulli packet loss model using Traffic Control attachment points.

For every packet processed, the module generates an independent probabilistic decision based on the configured packet loss percentage. Packets selected for dropping are discarded, while all remaining packets continue through the networking stack unchanged.

### Supported Attachment Points

- TC Ingress
- TC Egress

### Configuration Parameters

| Parameter                | Description                                                                    |
| ------------------------ | ------------------------------------------------------------------------------ |
| `packet_loss_percentage` | Probability of dropping a packet, expressed as a percentage between 0 and 100. |

### Example

```bash
load -m tc_bernoulli -i eth0 -a ingress

config packet_loss_percentage=10
```

This configuration causes approximately 10% of processed packets to be dropped.

---

## `xdp_bernoulli.bpf.c`

The `xdp_bernoulli` module implements the same Bernoulli packet loss model using XDP attachment points.

The packet selection algorithm is identical to the Traffic Control implementation, allowing direct behavioural comparisons across different execution environments.

### Supported Attachment Points

- XDP Generic
- XDP Native
- XDP Hardware Offload

### Configuration Parameters

| Parameter                | Description                                                                    |
| ------------------------ | ------------------------------------------------------------------------------ |
| `packet_loss_percentage` | Probability of dropping a packet, expressed as a percentage between 0 and 100. |

### Example

```bash
load -m xdp_bernoulli -i eth0 -a driver

config packet_loss_percentage=10
```

This configuration causes approximately 10% of processed packets to be dropped.

---

## Runtime Statistics

Both modules expose the same runtime statistics interface, allowing their behaviour to be compared under identical conditions.

The available metrics include:

- Total processed packets;
- Total dropped packets;
- Total execution time.

These statistics provide visibility into module behaviour and facilitate performance evaluation across different attachment points and execution modes.

<p>
  <a href="networking_stack.md">
    <img src="../../assets/images/buttons/prev_networking_stack.svg" align="left">
  </a>
  <a href="../development/developing_new_modules.md">
    <img src="../../assets/images/buttons/next_developing_new_modules.svg" align="right">
  </a>
</p>
