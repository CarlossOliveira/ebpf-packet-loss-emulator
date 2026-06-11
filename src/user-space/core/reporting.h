#ifndef REPORTING_H
#define REPORTING_H

#include "globals.h"

/**
 * @brief Dumps the statistics for the currently loaded eBPF module to the console and a log file.
 *
 * @param ctx A pointer to the application context containing the BPF and interface information.
 * @return 0 on success, -1 on failure.
 */
int dump_stats(const app_context_t *ctx);

#endif // !REPORTING_H
