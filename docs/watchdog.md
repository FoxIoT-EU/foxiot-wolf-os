# Watchdog

## Table of Contents

- [Overview](#overview)
- [How It Works](#how-it-works)
- [Default Setup](#default-setup)
- [Integrating Your Application](#integrating-your-application)
  - [C Integration](#c-integration)
- [Logging](#logging)
- [Behavior Summary](#behavior-summary)
- [Building from Source](#building-from-source)

---

## Overview

Wolf OS includes a watchdog system that automatically resets the controller if critical software stops responding. This ensures your device recovers from hangs or crashes without manual intervention — essential for unattended industrial deployments.

The system consists of two components:

- **`watchdogd`** — a daemon that manages the hardware watchdog (`/dev/watchdog`) and accepts keepalive messages from applications over a Unix socket
- **`wdt-kick`** — a command-line tool for sending keepalive messages to `watchdogd`

---

## How It Works

1. `watchdogd` opens the hardware watchdog and kicks it every 500ms
2. Applications register as **clients** by sending a message with their name and timeout
3. Each client must send periodic keepalives (kicks) within its timeout
4. If **any** registered client stops kicking, `watchdogd` stops kicking the hardware watchdog
5. The hardware watchdog then resets the system

If no clients are registered, `watchdogd` kicks the hardware watchdog unconditionally (backwards-compatible mode).

---

## Default Setup

The firmware starts `watchdogd` automatically via the `/etc/rc.wdt` script, which also registers a `system` client with a 30-second timeout and kicks it every 10 seconds:

```sh
#!/bin/sh
readonly NAME="system"
readonly TIMEOUT=30
readonly INTERVAL=10

/bin/watchdogd > /dev/null 2>&1 &

while true; do
    wdt-kick "$NAME" "$TIMEOUT"
    sleep "$INTERVAL"
done
```

This is launched from `rc.local`:

```sh
/etc/rc.wdt > /dev/null 2>&1 &
```

> 💡 The `system` client ensures a baseline watchdog is always active. If the init system itself hangs, the controller will reset.

For shell script integration, use the same `wdt-kick` pattern shown above in your own startup scripts. See [Adding Your Application to the Firmware](developer-quick-start.md#adding-your-application-to-the-firmware) for how to create startup scripts.

---

## Integrating Your Application

You can register your application as an additional watchdog client. If your application hangs or crashes and stops sending kicks, the system will reset automatically.

**Protocol:** Send a plain text datagram to `/var/run/watchdogd.sock` (Unix `SOCK_DGRAM`)

| Message | Description |
|---|---|
| `<name>` | Register with default timeout (120s), or kick if already registered |
| `<name> <timeout_sec>` | Register with custom timeout (seconds), or kick if already registered |

- `name` — unique identifier, max 31 characters, no spaces
- First message from a new name registers the client; subsequent messages are keepalives
- Max 8 concurrent clients

---

### C Integration

Persistent socket example for embedding in a C/C++ application. No dependencies beyond libc.

```c
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define WDT_SOCK_PATH "/var/run/watchdogd.sock"

static int wdt_fd = -1;
static struct sockaddr_un wdt_addr;

int wdt_init(void)
{
    wdt_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (wdt_fd < 0)
        return -1;

    memset(&wdt_addr, 0, sizeof(wdt_addr));
    wdt_addr.sun_family = AF_UNIX;
    strncpy(wdt_addr.sun_path, WDT_SOCK_PATH, sizeof(wdt_addr.sun_path) - 1);
    return 0;
}

int wdt_kick(const char *name, int timeout_sec)
{
    char buf[64];
    int len;

    if (wdt_fd < 0)
        return -1;

    if (timeout_sec > 0)
        len = snprintf(buf, sizeof(buf), "%s %d", name, timeout_sec);
    else
        len = snprintf(buf, sizeof(buf), "%s", name);
    if (len >= (int)sizeof(buf))
        len = sizeof(buf) - 1;

    return sendto(wdt_fd, buf, len, 0,
                  (struct sockaddr *)&wdt_addr, sizeof(wdt_addr)) > 0 ? 0 : -1;
}

void wdt_close(void)
{
    if (wdt_fd >= 0) {
        close(wdt_fd);
        wdt_fd = -1;
    }
}
```

Usage in your application:

```c
wdt_init();
wdt_kick("myapp", 60);  /* register with 60s timeout */

while (running) {
    /* ... do work ... */
    wdt_kick("myapp", 0);  /* periodic kick */
    sleep(30);
}

wdt_close();
```

---

## Logging

By default `watchdogd` logs to `stderr`. A log file can be enabled at runtime by sending a `log <path>` command. This is useful when persistent storage (e.g. SD card) is mounted after the daemon starts.

In the example distro, logging is enabled automatically after the SD card is mounted:

```sh
mkdir -p /mnt/sdcard/wdt
wdt-kick log /mnt/sdcard/wdt/wdt.log
```

**Log rotation:** When the log file exceeds 250KB it is renamed to `<path>.old` and a new file is created. Maximum disk usage is ~500KB.

**Logged events:**

| Event | Example |
|---|---|
| Logging started | `[2026-02-21 14:30:05] logging started` |
| Client registered | `[2026-02-21 14:30:05] registered client 'system' timeout=30s` |
| Client expired | `[2026-02-21 14:30:05] client 'system' expired — stopping WDT kicks` |
| HW kick failed | `[2026-02-21 14:30:05] failed to kick watchdog` |

> 💡 Individual client kicks are not logged to avoid excessive writes on flash storage.

---

## Behavior Summary

| Scenario | Result |
|---|---|
| No clients registered | HW watchdog kicked normally (backwards compatible) |
| All clients kicking on time | HW watchdog kicked normally |
| Any client misses its timeout | HW watchdog stops being kicked, system resets |
| `watchdogd` is killed (SIGKILL) | No one kicks HW watchdog, system resets |
| `watchdogd` is stopped (SIGTERM) | No magic close write, HW watchdog resets the system |
| Socket unavailable | `watchdogd` runs without socket (HW watchdog only) |

---

## Building from Source

The watchdog source is located in `util/watchdog/`. To rebuild:

```sh
cd util/watchdog
make clean
make
```

This produces `watchdogd` and `wdt-kick` binaries cross-compiled for the Wolf controller. Copy them to `base/packages/list/base/root/bin/` to include in firmware builds.

> 💡 Requires `arm-linux-gnueabi-gcc` cross-compiler. See [Building with Docker](docker-build.md) for the recommended build environment.

---

⬅️ [Back to main page](../README.md)
