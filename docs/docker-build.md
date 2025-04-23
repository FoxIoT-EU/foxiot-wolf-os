# Building FoxIoT Gateway OS with Docker

This guide describes how to build the **FoxIoT Gateway OS** for the **FoxIoT Wolf IIoT Gateway** using **Docker**. This is the **recommended method** to ensure consistent and reproducible builds across different systems.

---

## ✅ Requirements

- [Docker Engine](https://docs.docker.com/engine/install/) installed and running
- Internet access during the first build

---

## Usage

### Build a specific distro

```sh
./build-in-docker.sh foxiot
```

### Clean build artifacts

```sh
./build-in-docker.sh --clean foxiot
```

### Rebuild the Docker image

```sh
./build-in-docker.sh --rebuild-image foxiot
```

### List available distros

```sh
./build-in-docker.sh --list-distros
```

You can combine options as well:

```sh
./build-in-docker.sh --clean --rebuild-image foxiot
```

---

## Output

Build results are placed in the `build/` directory under the project root. The folder structure can vary depending on the distro and your Makefile.

---

## File Permissions

The Docker container runs using your current user ID (`UID:GID`), so files created during the build process are owned by you — no `sudo` needed.

---

## Need Help?

Contact [support@foxiot.eu](mailto:support@foxiot.eu).

---
⬅️ [Back to main page](../README.md)
