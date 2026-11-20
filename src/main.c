#define _POSIX_C_SOURCE 200809L

#include "edge_health/metrics.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef EDGE_HEALTH_VERSION
#define EDGE_HEALTH_VERSION "dev"
#endif

static volatile sig_atomic_t keep_running = 1;

static void stop_handler(int signal_number) {
    (void)signal_number;
    keep_running = 0;
}

static void print_usage(FILE *stream, const char *program) {
    fprintf(stream,
            "Usage: %s [--once] [--interval SECONDS] [--help] [--version]\n",
            program);
}

static int parse_interval(const char *value, unsigned int *interval) {
    char *end = NULL;
    errno = 0;
    const unsigned long parsed = strtoul(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0' || parsed == 0 || parsed > 86400) {
        return -1;
    }
    *interval = (unsigned int)parsed;
    return 0;
}

int main(int argc, char **argv) {
    int once = 0;
    unsigned int interval = 30;

    for (int index = 1; index < argc; ++index) {
        if (strcmp(argv[index], "--once") == 0) {
            once = 1;
        } else if (strcmp(argv[index], "--interval") == 0 && index + 1 < argc) {
            if (parse_interval(argv[++index], &interval) != 0) {
                fprintf(stderr, "Invalid interval: %s\n", argv[index]);
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[index], "--help") == 0) {
            print_usage(stdout, argv[0]);
            return EXIT_SUCCESS;
        } else if (strcmp(argv[index], "--version") == 0) {
            puts(EDGE_HEALTH_VERSION);
            return EXIT_SUCCESS;
        } else {
            print_usage(stderr, argv[0]);
            return EXIT_FAILURE;
        }
    }

    (void)signal(SIGINT, stop_handler);
    (void)signal(SIGTERM, stop_handler);

    do {
        struct edge_metrics metrics;
        char json[256];
        if (edge_metrics_read("/proc", "/sys", &metrics) != 0 ||
            edge_metrics_to_json(&metrics, json, sizeof(json)) < 0) {
            perror("edge-health-agent");
            return EXIT_FAILURE;
        }

        puts(json);
        fflush(stdout);
        if (once) {
            break;
        }

        struct timespec remaining = {.tv_sec = (time_t)interval, .tv_nsec = 0};
        while (keep_running && nanosleep(&remaining, &remaining) != 0 && errno == EINTR) {
        }
    } while (keep_running);

    return EXIT_SUCCESS;
}
