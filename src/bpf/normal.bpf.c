#include "globals.h"

#include "helpers.bpf.h"

#include "math_utils.bpf.h"

// PARAMETERS: "standard_deviation", "mean"

char LICENSE[] SEC("license") = "GPL";

SEC("classifier")
int packet_handler(struct __sk_buff *skb) {
  (void)skb;

  __u64 start_time = bpf_ktime_get_ns();
  __u8 key;
  __u64 value;

  int ret = TC_ACT_OK;

  __u64 standard_deviation = get_config_value("standard_deviation");
  __u64 mean = get_config_value("mean");

  if (1) {
    key = PACKETS_DROPPED;
    value = get_stats(key) + 1;
    update_stats(key, value);
    ret = TC_ACT_SHOT;
  }

  key = PACKETS_PROCESSED;
  value = get_stats(key) + 1;
  update_stats(key, value);

  key = TOTAL_EXECUTION_TIME;
  value = get_stats(key) + (bpf_ktime_get_ns() - start_time);
  update_stats(key, value);

  return ret;
}