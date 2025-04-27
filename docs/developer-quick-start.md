# Developer Quick Start

## Table of Contents

- [1. Accessing the Controller](#1-accessing-the-controller)
- [2. Compiling Your First Application](#2-compiling-your-first-application)
- [3. Uploading Your Application](#3-uploading-your-application)
- [4. Running Your Application](#4-running-your-application)

This guide helps you quickly start working with your FoxIoT Wolf controller after installing the firmware.

---

## 1. Accessing the Controller

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

## 2. Compiling Your First Application

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



