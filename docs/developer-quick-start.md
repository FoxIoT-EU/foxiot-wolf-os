# Developer Quick Start

## Table of Contents

- [Accessing the Controller](#accessing-the-controller)
- [Compiling Your First Application](#compiling-your-first-application)
- [Adding Your Application to the Firmware](#adding-your-application-to-the-firmware)
- [Configuring netd (Network Daemon)](#configuring-netd-network-daemon)

This guide helps you quickly start working with your FoxIoT Wolf controller after installing the firmware.

---

## Accessing the Controller

### Quick Links
- [Finding the Controller's IP Address](#finding-the-controllers-ip-address)
- [Logging in via SSH](#logging-in-via-ssh)
- [Default Root Password and Security Settings](#default-root-password-and-security-settings)
- [Adding an SSH Public Key](#adding-an-ssh-public-key)

---

### Finding the Controller's IP Address

Once your controller is running, it will automatically try to obtain an IP address via DHCP by default. (You can later configure a static IP if needed.)

To find the controller on your network:
- Check your router's DHCP client list.
- Use `arp-scan` to discover new devices:

```bash
# Find your network interface (e.g., eth0, wlan0):
ip addr

# Scan your local network:
sudo arp-scan --interface=eth0 --localnet
```

Or use `nmap` if you prefer:

```bash
# Find your local subnet from your IP address (e.g., 192.168.1.123/24 → subnet is 192.168.1.0/24)
# Then scan the network:
nmap -sn 192.168.1.0/24
```

---

### Logging in via SSH

After finding the IP address, access the controller over SSH:

```bash
ssh root@YOUR_CONTROLLER_IP
```

- Default user: `root`
- Default password: `foxiot`

---

### Default Root Password and Security Settings

The root password is managed through your project Makefile (located at `distro/YOUR_PROJECT_NAME/Makefile`):

```make
PASSWORD = "foxiot"
```

> 💡 **Important Security Note:**
>
> To change the root password, edit this line before building the firmware.
>
> To **disable password login completely**, comment it out by adding a `#`:
>
> ```make
> # PASSWORD = "foxiot"
> ```
>
> It is highly recommended to disable root password login and rely only on SSH public key authentication for accessing the controller.

---

### Adding an SSH Public Key

For secure, passwordless login:

1. Create an SSH key on your computer (if you don't have one):
   ```bash
   ssh-keygen -t ed25519
   ```

2. Add your public key(s) to the firmware project before building:

   Use your preferred text editor to create or edit the file, and place one or more public keys into it:

   ```bash
   distro/YOUR_PROJECT_NAME/root/etc/dropbear/authorized_keys
   ```

> 💡 You can add multiple public keys (one per line) if you want to allow access for several users.
> These keys will be automatically included into the firmware, allowing secure, passwordless access after flashing.

---

## Compiling Your First Application

You will need to cross-compile your application for the controller's CPU architecture.

The FoxIoT Wolf controller uses an ARM926EJ-S processor (ARMv5 architecture). Make sure to target ARMv5 when building your applications.

### Quick Links
- [C Example](#example-c-hello-world-program)
- [C++ Example](#example-cpp-hello-world-program)
- [Rust Example](#example-rust-hello-world-program)

---

### Example: C Hello World Program

Create a simple C file:

```c
// hello.c
#include <stdio.h>

int main() {
    printf("Hello from Wolf Controller!\n");
    return 0;
}
```

Compile it for the target architecture (adjust your cross-compiler prefix accordingly):

```bash
arm-linux-gnueabi-gcc hello.c -o hello_world
```

You can optionally strip the binary to reduce its size:

```bash
arm-linux-gnueabi-strip hello_world
```

---

### Example: Cpp Hello World Program

Create a simple C++ file:

```cpp
// hello.cpp
#include <iostream>

int main() {
    std::cout << "Hello from Wolf Controller!" << std::endl;
    return 0;
}
```

Compile it for the target architecture:

```bash
arm-linux-gnueabi-g++ hello.cpp -o hello_world_cpp
```

You can optionally strip the binary to reduce its size:

```bash
arm-linux-gnueabi-strip hello_world_cpp
```

---

### Example: Rust Hello World Program

> 💡 To optimize your Rust binary size further, you can add the following to your `Cargo.toml` under the `[profile.release]` section:
> 
> ```toml
> [profile.release]
> strip = true
> opt-level = "z"
> lto = true
> codegen-units = 1
> ```
> 
> This will strip the binary, apply size optimizations, enable Link Time Optimization (LTO), and use a single codegen unit for smaller output.

Create a new Rust project (make sure Rust is installed):

```bash
cargo new hello_wolf
cd hello_wolf
```

Edit `src/main.rs` if needed:

```rust
fn main() {
    println!("Hello from Wolf Controller!");
}
```

Cross-compile it for the target architecture (setup for cross-compiling may be needed separately):

```bash
cargo build --release --target=armv5te-unknown-linux-gnueabi
```

The compiled binary will appear under:

```bash
target/armv5te-unknown-linux-gnueabi/release/hello_wolf
```

---

## Adding Your Application to the Firmware

After compiling your application, you can include it directly into your project build.

### Steps:

1. (Recommended) Go to your project directory:

   If you don't go into the project directory, you will need to use full paths like `distro/YOUR_PROJECT_NAME/root/bin/` for all file operations.

   ```bash
   cd distro/YOUR_PROJECT_NAME
   ```

2. Copy your application binary into the `root/bin/` folder:

   ```bash
   cp PATH_TO_YOUR_APP_BINARY root/bin/
   ```

3. Tell the builder to include the binary in the firmware:

   Open `rootfs.list`, find the existing `dir /bin 755 0 0` line,
   and add your file line directly after it:

   ```text
   file /bin/YOUR_APP_NAME root/bin/YOUR_APP_NAME 755 0 0
   ```

4. (Optional) Create a startup script if you want the app to start automatically:

   Create a new file `root/etc/rc.YOUR_APP_NAME`:

   ```bash
   #!/bin/sh
   while true; do
       /bin/YOUR_APP_NAME >/dev/null 2>&1
       sleep 1
   done
   ```

   Also add your startup script into `rootfs.list` after the existing `/etc` folder:

   ```text
   file /etc/rc.YOUR_APP_NAME root/etc/rc.YOUR_APP_NAME 755 0 0
   ```

5. Add the startup script to `rc.local` to run it on boot:

Edit `root/etc/rc.local` and insert the following line **before** the `exit 0` line:

```bash
# Start YOUR_APP_NAME
/etc/rc.YOUR_APP_NAME > /dev/null 2>&1 &
```

6. Build and install the updated firmware:

   Follow the instructions from the [Getting Started Guide - Build the Image and Install](../docs/getting-started.md#4-build-the-image) section to rebuild and flash the firmware to your controller.

---

## Configuring netd (Network Daemon)

The `netd` service is FoxIoT's network management daemon responsible for:

- Managing Ethernet and modem connections
- Assigning IP addresses (DHCP client or static)
- Setting up network routes
- Handling priority between Ethernet and mobile modems (failover support)

`netd` requires a JSON configuration file to operate.
By default, the `/etc/rc.netd` startup script loads the configuration from `/mnt/rodata/netd/netd.conf`. If this file does not exist, it will fall back to `/etc/netd.conf`.

---

### Quick Links

- [Full Example Configuration](#full-example-configuration)
- [Switching LAN to DHCP](#switching-lan-to-dhcp)
- [Interfaces](#interfaces)
- [Priorities](#priorities)
- [Ping Monitoring](#ping-monitoring)

---

## Configuration (JSON Format)

The configuration defines how network interfaces are managed, how priorities are handled, and how network failover is processed.

### Full Example Configuration

```json
{
  "actions": {
    "down": { "type": "down" },
    "up": { "type": "up" }
  },
  "interfaces": {
    "lan": {
      "configurations": {
        "static": {
          "ping": {
            "error": 3,
            "ip": "1.1.1.1",
            "interval": 300
          },
          "mode": "static",
          "netconf": {
            "address": "192.168.2.132",
            "dns": "1.1.1.1",
            "dns2": "8.8.8.8",
            "gateway": "192.168.2.1",
            "netmask": "255.255.255.0"
          }
        }
      },
      "device": { "name": "eth0" },
      "type": "lan"
    },
    "wwan": {
      "configurations": {
        "lte": {
          "apn": "terminal",
          "ping": {
            "error": 3,
            "ip": "1.1.1.1",
            "interval": 300
          }
        }
      },
      "device": {
        "disable": "gpiodset pcie_power off",
        "enable": "gpiodset pcie_power on",
        "usbDevice": "1-2"
      },
      "type": "rndis"
    }
  },
  "priorities": {
    "lan": {
      "configuration": "static",
      "priority": 0
    },
    "wwan": {
      "configuration": "lte",
      "priority": 1
    }
  }
}
```

### Switching LAN to DHCP

If you want to configure the LAN interface to use DHCP instead of a static IP address:

Replace the `"static"` configuration inside the LAN `"configurations"` block with the following "dynamic" configuration:

```json
"dynamic": {
  "mode": "dynamic"
}
```
After this change, the full `"configurations"` section for LAN should look like:

```json
"configurations": {
  "dynamic": {
    "mode": "dynamic"
  }
}
```

This will instruct `netd` to request an IP address automatically using DHCP for the Ethernet interface.

---

## Configuration Structure Explained

### Interfaces

Defines the network interfaces and their basic settings.

#### LAN Interface (Ethernet)

- Type: `lan`
- Device: Typically `eth0`.
- Configuration Modes:
  - `static`: Manual IP address configuration.
  - `dynamic`: DHCP client mode.
- Optional `ping` monitoring can be added to check internet connectivity health.
- Key fields for LAN:
  - `mode`: Connection mode (`static` or `dynamic`) for LAN interfaces only.
  - `netconf`: Static IP settings (address, gateway, DNS, netmask).
  - `ping`: *(Optional)* Enables connection monitoring.

#### WWAN Interface (Modem)

- Type: `rndis` (uses RNDIS USB driver for modem connection)
- Configuration defines mobile network connection parameters.
- `ping` monitoring is **mandatory** for reliability.

Key fields for WWAN:
  - `apn`: Required, must be set according to your mobile operator.
  - `ping`: *(Mandatory)* Monitors internet connectivity.

### Priorities

- Defines the order of interface usage.
- **Lower number = higher priority**.
- netd will try to use the highest-priority interface that is reachable.
- If the connection is lost (based on ping failure) or the network link goes down (e.g., Ethernet cable unplugged), it switches to the next available interface.



### Ping Monitoring

If the monitored interface (LAN or WWAN) fails to respond to the defined ping checks, netd will mark the connection as disconnected. In case of modem (WWAN) failure, netd will also automatically restart the modem to attempt recovery.

- `ip`: IP address to ping (to check if internet is reachable).
- `interval`: Time interval between pings (in seconds).
- `error`: Number of consecutive ping failures before marking the interface as disconnected.

---

