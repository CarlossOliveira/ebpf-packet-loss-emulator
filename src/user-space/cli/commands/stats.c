#include "stats.h"

#include "globals.h"

#include "commands.h"
#include "io_utils.h"
#include "time_utils.h"

#include <bpf/bpf.h>
#include <libgen.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

static uint64_t read_stats_map(int map_fd, uint8_t key);
static int dump_to_log_file(const char *filename, const char *data);

void stats_command(app_context_t *ctx, char **input)
{
	if (input)
		if (HELP_FLAG) {
			print(NULL, STATS_HELP_MESSAGE);
			return;
		}

	if (strlen(ctx->bpf.module_name) == 0) {
		print(WARNING, "No module loaded");
		return;
	}

	dump_stats(ctx);
}

int dump_stats(app_context_t *ctx)
{
	char stats[1024];
	char time_buffer[20];

	get_current_time(time_buffer, sizeof(time_buffer));
	uint64_t packets_processed = 0;
	uint64_t packets_dropped = 0;
	uint64_t total_execution_time = 0;
	if (ctx->bpf.maps.stats_map_fd > 0) {
		packets_processed = read_stats_map(ctx->bpf.maps.stats_map_fd, PACKETS_PROCESSED);
		packets_dropped = read_stats_map(ctx->bpf.maps.stats_map_fd, PACKETS_DROPPED);
		total_execution_time = read_stats_map(ctx->bpf.maps.stats_map_fd, TOTAL_EXECUTION_TIME);
	}

	snprintf(stats, sizeof(stats),
		 "---------- STATS ----------\n[%s]\nBPF Module: "
		 "%s\nPackets "
		 "processed: %lu\nPackets dropped: %lu\nTotal "
		 "execution time: %lu "
		 "ns\n---------------------------",
		 time_buffer, strlen(ctx->bpf.module_name) > 0 ? ctx->bpf.module_name : "Module not loaded",
		 packets_processed, packets_dropped, total_execution_time);
	printf("%s", BLUE);
	print(NULL, "%s", stats);
	printf("%s", RESET);

	snprintf(stats, sizeof(stats),
		 "{\"module\":\"%s\","
		 "\"timestamp\":\"%s\","
		 "\"packets_processed\":%lu,"
		 "\"packets_dropped\":%lu,"
		 "\"total_execution_time\":%lu}",
		 strlen(ctx->bpf.module_name) > 0 ? ctx->bpf.module_name : "Module not loaded", time_buffer,
		 packets_processed, packets_dropped, total_execution_time);

	return dump_to_log_file(LOG_FILE, stats);
}

static uint64_t read_stats_map(int map_fd, uint8_t key)
{
	uint64_t value = 0;

	if (map_fd < 0)
		return 0;

	if (bpf_map_lookup_elem(map_fd, &key, &value) != 0)
		return 0;

	return value;
}

static int dump_to_log_file(const char *filename, const char *data)
{
	FILE *file = fopen(filename, "a");

	if (!file) {
		// Extract directory from filename
		char path[AF_MAX];
		snprintf(path, sizeof(path), "%s", filename);

		// Create directory if it doesn't exist
		char *dir = dirname(path);
		if (mkdir(dir, 0755) != 0 && errno != EEXIST) {
			print(ERROR, "Failed to create log directory");
			return 1;
		}

		// Try opening the file again
		file = fopen(filename, "a");

		if (!file) {
			print(ERROR, "Failed to open log file");
			return 1;
		}
	}

	fprintf(file, "%s\n", data);
	fclose(file);

	return 0;
}
