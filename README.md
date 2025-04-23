# FoxIoT Gateway OS Builder

Welcome to the FoxIoT Gateway OS build system. This repository helps you build and customize firmware for the FoxIoT Wolf IIoT Gateway.

## Getting Started

Make sure [Git](https://git-scm.com/downloads) is installed.

Then clone this repository:

```sh
git clone https://github.com/FoxIoT-EU/foxiot-wolf-os.git
cd foxiot-wolf-os
```

## Supported Build Methods

You can build the OS in two ways:

- **[Using Docker (recommended)](docs/docker-build.md)** – the preferred method for consistent and reproducible builds across different systems and teams.
- **[Locally on Linux](docs/local-build.md)** – build directly on your system using native tools (documentation coming soon).

## Repository Structure

```
foxiot-wolf-os/
├── base/                       # Core packages, common device tree, kernel image, and build logic
├── docker/
│   ├── Dockerfile
│   └── make-inside-docker.sh   # Internal script used in container
├── distro/
│   ├── foxiot/
│   └── other-distros/
├── build/                      # Output directory
└── build-in-docker.sh          # Entry point for Docker-based builds
```

## Support

If you have any questions or run into issues, please reach out to [support@foxiot.eu](mailto:support@foxiot.eu).

## Third-Party Licenses

This project includes or depends on third-party software components (such as BusyBox, Dropbear, etc.)  
which are distributed under their own respective licenses (e.g., GPLv2, MIT, BSD).

These components are not covered by this repository’s MIT License.  
Please refer to the documentation or upstream sources of each component for licensing details.

We do not modify these packages and include them as-is in the build system.

