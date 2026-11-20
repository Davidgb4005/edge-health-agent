#include "edge_health/metrics.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

static int read_uptime(const char *proc_root, double *uptime_seconds) {
    char path[512];
    if (snprintf(path, sizeof(path), "%s/uptime", proc_root) >= (int)sizeof(path)) {
        return -1;
    }

    FILE *stream = fopen(path, "r");
    if (stream == NULL) {
        return -1;
    }

    const int fields = fscanf(stream, "%lf", uptime_seconds);
    const int close_result = fclose(stream);
    return fields == 1 && close_result == 0 ? 0 : -1;
}

static int read_memory(const char *proc_root,
                       unsigned long *total_kib,
                       unsigned long *available_kib) {
    char path[512];
    if (snprintf(path, sizeof(path), "%s/meminfo", proc_root) >= (int)sizeof(path)) {
        return -1;
    }

    FILE *stream = fopen(path, "r");
    if (stream == NULL) {
        return -1;
    }

    char key[64];
    unsigned long value = 0;
    char unit[16];
    int found_total = 0;
    int found_available = 0;

    while (fscanf(stream, "%63s %lu %15s", key, &value, unit) == 3) {
        if (strcmp(key, "MemTotal:") == 0) {
            *total_kib = value;
            found_total = 1;
        } else if (strcmp(key, "MemAvailable:") == 0) {
            *available_kib = value;
            found_available = 1;
        }
    }

    const int close_result = fclose(stream);
    return found_total && found_available && close_result == 0 ? 0 : -1;
}

static void read_temperature(const char *sys_root, struct edge_metrics *metrics) {
    char path[512];
    if (snprintf(path,
                 sizeof(path),
                 "%s/class/thermal/thermal_zone0/temp",
                 sys_root) >= (int)sizeof(path)) {
        return;
    }

    FILE *stream = fopen(path, "r");
    if (stream == NULL) {
        return;
    }

    long millidegrees = 0;
    if (fscanf(stream, "%ld", &millidegrees) == 1) {
        metrics->temperature_celsius = (double)millidegrees / 1000.0;
        metrics->has_temperature = 1;
    }
    (void)fclose(stream);
}

int edge_metrics_read(const char *proc_root,
                      const char *sys_root,
                      struct edge_metrics *metrics) {
    if (proc_root == NULL || sys_root == NULL || metrics == NULL) {
        errno = EINVAL;
        return -1;
    }

    *metrics = (struct edge_metrics){0};
    if (read_uptime(proc_root, &metrics->uptime_seconds) != 0 ||
        read_memory(proc_root,
                    &metrics->memory_total_kib,
                    &metrics->memory_available_kib) != 0) {
        return -1;
    }

    read_temperature(sys_root, metrics);
    return 0;
}

int edge_metrics_to_json(const struct edge_metrics *metrics,
                         char *buffer,
                         size_t buffer_size) {
    if (metrics == NULL || buffer == NULL || buffer_size == 0) {
        errno = EINVAL;
        return -1;
    }

    int written;
    if (metrics->has_temperature) {
        written = snprintf(buffer,
                           buffer_size,
                           "{\"uptime_seconds\":%.2f,\"memory_total_kib\":%lu,"
                           "\"memory_available_kib\":%lu,\"temperature_celsius\":%.2f}",
                           metrics->uptime_seconds,
                           metrics->memory_total_kib,
                           metrics->memory_available_kib,
                           metrics->temperature_celsius);
    } else {
        written = snprintf(buffer,
                           buffer_size,
                           "{\"uptime_seconds\":%.2f,\"memory_total_kib\":%lu,"
                           "\"memory_available_kib\":%lu,\"temperature_celsius\":null}",
                           metrics->uptime_seconds,
                           metrics->memory_total_kib,
                           metrics->memory_available_kib);
    }

    return written >= 0 && (size_t)written < buffer_size ? written : -1;
}
