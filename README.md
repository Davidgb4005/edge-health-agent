# Edge Health Agent

A small C service for collecting Linux device-health metrics on constrained edge
systems. It reads kernel interfaces directly, emits newline-delimited JSON, and
includes a hardened `systemd` unit plus a Yocto integration layer.

## Why this project exists

Embedded Linux products often need a low-overhead way to expose basic health
data without pulling in a large runtime. This agent demonstrates the pieces that
matter in production firmware: defensive parsing, signal-safe shutdown,
cross-compilation-friendly CMake, deterministic tests, service hardening, and
packaging for an immutable target image.

## Metrics

- Uptime from `/proc/uptime`
- Total and available memory from `/proc/meminfo`
- CPU/board temperature from thermal sysfs when available

Example output:

```json
{"uptime_seconds":1234.50,"memory_total_kib":1024000,"memory_available_kib":768000,"temperature_celsius":42.12}
```

## Build and test

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/edge-health-agent --once
```

Run continuously with a custom sample interval:

```sh
./build/edge-health-agent --interval 10
```

## Embedded Linux integration

The companion [`meta-edge-demo`](https://github.com/Davidgb4005/meta-edge-demo)
layer builds this project with BitBake, installs the service, and produces a
QEMU ARM64 image. CMake honors the toolchain supplied by Yocto; the program has
no dependencies outside the C library.

## Design notes

- Reads through small, testable functions instead of shelling out to utilities.
- Treats temperature as optional because not every kernel exposes a thermal zone.
- Uses fixture-backed tests so parsing is repeatable on hosts and in CI.
- Runs as a dynamic unprivileged user with a read-only filesystem under systemd.

## License

MIT
