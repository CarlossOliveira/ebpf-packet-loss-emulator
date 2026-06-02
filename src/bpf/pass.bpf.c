#include "../include/globals.h"
#include "../include/bpf_utils.bpf.h"

char LICENSE[] SEC("license") = "GPL";

SEC("classifier")
int packet_handler(struct __sk_buff *skb)
{
    (void)skb;

    __u64 start_time = bpf_ktime_get_ns();
    __u32 key;
    __u64 value;

    key = PACKETS_PROCESSED;
    value = get_stats(key) + 1;
    update_stats(key, value);

    key = TOTAL_EXECUTION_TIME;
    value = get_stats(key) + (bpf_ktime_get_ns() - start_time);
    update_stats(key, value);

    return TC_ACT_OK;
}