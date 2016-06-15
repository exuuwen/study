#include "log.h"


RTE_DEFINE_PER_LCORE(struct trace_log, g_trace_log) = {LOG_INFO, false, {0, 0, 0, 0, 0}};
