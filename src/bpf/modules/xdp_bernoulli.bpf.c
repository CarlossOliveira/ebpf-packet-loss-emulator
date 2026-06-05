#include "globals.h"

#include "helpers.bpf.h"

#include "math_utils.bpf.h"

char LICENSE[] SEC("license") = "GPL";

char CONFIG_KEYS[][CONFIG_KEY_SIZE] SEC(".config_keys") = {
    "packet_loss_percentage", {0}, // Sentinel to mark the end of valid keys
};

SEC("xdp")
int packet_handler(struct xdp_md *ctx)
{
	(void)ctx;

	__u64 start_time = bpf_ktime_get_ns();
	__u8 key;
	__u64 value;

	int ret = XDP_PASS;

	char percentage_key[CONFIG_KEY_SIZE] = "packet_loss_percentage";
	__u64 percentage = get_config_value(percentage_key);

	if (percentage > 100)
		percentage = 100;

	if ((bpf_get_prandom_u32() % 100) < percentage) {
		key = PACKETS_DROPPED;
		value = get_stats(key) + 1;
		update_stats(key, value);
		ret = XDP_DROP;
	}

	key = PACKETS_PROCESSED;
	value = get_stats(key) + 1;
	update_stats(key, value);

	key = TOTAL_EXECUTION_TIME;
	value = get_stats(key) + (bpf_ktime_get_ns() - start_time);
	update_stats(key, value);

	return ret;
}