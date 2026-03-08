# Firmware Signing

## Table of Contents

- [Overview](#overview)
- [Setup](#setup)
  - [Building the Tools](#building-the-tools)
  - [Generating Keys](#generating-keys)
  - [Configuring Your Distro](#configuring-your-distro)
- [How It Works](#how-it-works)
- [Tools](#tools)
- [Verification on the Device](#verification-on-the-device)
- [Trailer Format](#trailer-format)
- [Algorithm](#algorithm)
- [Threat Model](#threat-model)

---

## Overview

Wolf OS requires firmware signing with Ed25519 to verify authenticity before flashing. This protects against tampered firmware files (MITM, compromised update server).

Every distro must set `FW_KEY` in its Makefile — the build will fail without it. On the device, the `install` command verifies the signature before flashing — rejecting any firmware that is unsigned, tampered, or signed with a different key.

---

## Setup

### Building the Tools

The signing tools are in `util/fw-sign/`. First, initialize the Monocypher submodule and build:

```bash
git submodule update --init
cd util/fw-sign
make host       # builds fw-sign + fw-keygen (for your build machine)
make target     # builds fw-verify (ARM cross-compile for the device)
make            # both
```

### Generating Keys

Generate an Ed25519 keypair for your project:

```bash
mkdir -p fw_keys/YOUR_PROJECT
util/fw-sign/fw-keygen fw_keys/YOUR_PROJECT
```

This creates:
- `fw_keys/YOUR_PROJECT/secret.key` (64 bytes, mode 0600) — stays on build machine only
- `fw_keys/YOUR_PROJECT/public.key` (32 bytes) — deployed to devices

Both files are raw binary (not PEM, not hex).

> 💡 **Keep the secret key safe.** Anyone with the secret key can sign firmware that your devices will accept. Never commit it to a public repository.

### Configuring Your Distro

Three steps to configure signing for your distro (required before building):

1. **Set `FW_KEY` in your distro Makefile:**

   ```make
   FW_KEY = ../../fw_keys/YOUR_PROJECT/secret.key
   ```

2. **Deploy the public key to the device.** Copy the public key:

   ```bash
   cp fw_keys/YOUR_PROJECT/public.key distro/YOUR_PROJECT/root/etc/fw-verify.pub
   ```

3. **Add the public key to `rootfs.list`:**

   ```text
   file /etc/fw-verify.pub root/etc/fw-verify.pub 644 0 0
   ```

After this, `./build-in-docker.sh YOUR_PROJECT` will automatically sign the firmware, and the device will verify it before flashing.

---

## How It Works

**Build time:**
1. Firmware `.itb` image is built normally
2. `fw-sign` appends an 82-byte signature trailer to the image (build fails if `FW_KEY` is not set)
3. The original firmware data is not modified

**On the device:**
1. User runs `install /tmp/firmware.itb`
2. `install` checks that `/etc/fw-verify.pub` exists
3. `fw-verify` checks the signature against the public key
4. Only if verification passes, the firmware is flashed

---

## Tools

All tools are in `util/fw-sign/`.

| Tool | Runs on | Purpose |
|------|---------|---------|
| `fw-keygen` | Build machine | Generate Ed25519 keypair |
| `fw-sign` | Build machine | Sign firmware (append trailer) |
| `fw-verify` | Device (ARM) | Verify firmware before flashing |

---

## Verification on the Device

The `install` command automatically verifies firmware before flashing. You can also verify manually:

```bash
fw-verify /tmp/firmware.itb /etc/fw-verify.pub
```

Exit code 0 means valid, 1 means rejected. Output examples:

```
fw-verify: valid (key_id=a3f1c0b2, signed=2026-03-06 14:30:00, size=4521984)
fw-verify: FAILED - signature mismatch
fw-verify: FAILED - not a signed firmware (no WSGN magic)
fw-verify: FAILED - key_id mismatch (firmware=a3f1c0b2, device=7e02d1ff)
```

---

## Trailer Format

The signature trailer is appended to the end of the firmware file (82 bytes):

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0 | 1 | version | `0x01` |
| 1 | 1 | flags | `0x00` (reserved) |
| 2 | 4 | key_id | First 4 bytes of public key |
| 6 | 4 | timestamp | Signing time (uint32 BE, unix epoch) |
| 10 | 4 | fw_size | Original firmware size (uint32 BE) |
| 14 | 64 | signature | Ed25519ph signature |
| 78 | 4 | magic | `WSGN` |

The magic bytes `WSGN` are always at the very end of the file. Signing the same file twice is rejected.

---

## Algorithm

- Ed25519ph (pre-hash variant) with Monocypher library
- Firmware is streamed through SHA-512 in 4KB blocks (no full file load)
- The trailer metadata (14 bytes before signature) is included in the hash
- `fw-verify` uses ~4.5 KB stack, zero heap allocation

---

## Threat Model

- **Protects against:** tampered firmware files, MITM attacks, compromised update server
- **Does NOT protect against:** physical attacker with flash access (no secure boot on NUC980)
- Verification is userspace only (bootloader has no secure boot support)

---

⬅️ [Back to main page](../README.md)
