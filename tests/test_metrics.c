#include "edge_health/metrics.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIXTURE_ROOT
#error "FIXTURE_ROOT must be defined"
#endif

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "check failed at %s:%d: %s\n",                    \
                    __FILE__, __LINE__, #condition);                            \
            return EXIT_FAILURE;                                                \
        }                                                                       \
    } while (0)

int main(void) {
    struct edge_metrics metrics;
    char proc_root[512];
    char sys_root[512];
    CHECK(snprintf(proc_root, sizeof(proc_root), "%s/proc", FIXTURE_ROOT) > 0);
    CHECK(snprintf(sys_root, sizeof(sys_root), "%s/sys", FIXTURE_ROOT) > 0);

    CHECK(edge_metrics_read(proc_root, sys_root, &metrics) == 0);
    CHECK(metrics.uptime_seconds == 1234.50);
    CHECK(metrics.memory_total_kib == 1024000UL);
    CHECK(metrics.memory_available_kib == 768000UL);
    CHECK(metrics.has_temperature == 1);
    CHECK(metrics.temperature_celsius == 42.125);

    char json[256];
    CHECK(edge_metrics_to_json(&metrics, json, sizeof(json)) > 0);
    CHECK(strstr(json, "\"uptime_seconds\":1234.50") != NULL);
    CHECK(strstr(json, "\"memory_available_kib\":768000") != NULL);
    CHECK(strstr(json, "\"temperature_celsius\":42.12") != NULL);

    CHECK(edge_metrics_read(NULL, sys_root, &metrics) == -1);
    CHECK(edge_metrics_to_json(&metrics, json, 8) == -1);
    return EXIT_SUCCESS;
}
