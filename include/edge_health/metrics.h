#ifndef EDGE_HEALTH_METRICS_H
#define EDGE_HEALTH_METRICS_H

#include <stddef.h>

struct edge_metrics {
    double uptime_seconds;
    unsigned long memory_total_kib;
    unsigned long memory_available_kib;
    double temperature_celsius;
    int has_temperature;
};

int edge_metrics_read(const char *proc_root,
                      const char *sys_root,
                      struct edge_metrics *metrics);

int edge_metrics_to_json(const struct edge_metrics *metrics,
                         char *buffer,
                         size_t buffer_size);

#endif
