# Cellular Module Firmware Update

The Wolf Gateway's optional LTE module (Quectel EC25-EUX, mini-PCIe) runs its own firmware. Wolf-OS provides Quectel's flashing tool `QFirehose` as the package `qfirehose` so that the module firmware can be replaced through the gateway, without opening the enclosure. Include the package in your distro (`#include "qfirehose.list"` in `rootfs.list`), or copy the 93 KB binary from `base/packages/list/qfirehose/root/bin/QFirehose` to the unit over SSH when an update is due (`scp … root@<gateway>:/tmp/` and run it from `/tmp`; the root filesystem is a RAM overlay, so the copy is gone after the next reboot).

This page describes the procedure, its constraints and how to verify the result.

## What you need

- A Quectel firmware package for the EC25-EUX (a directory with `contents.xml`, `md5.txt` and an `update/` tree containing `prog_nand_firehose_9x07.mbn`, `rawprogram_nand_*.xml` and the `.mbn`/`.img`/`.ubi` images). Packages are obtained from Quectel or the module supplier; FoxIoT publishes the release it has qualified together with its SHA-256 hash. Use only packages for the **EC25-EUX** variant.
- A PC on the same LAN as the gateway, or reachable through the gateway's WireGuard tunnel, with `QFirehose` built natively (`base/packages/list/qfirehose/build.sh` builds the gateway binary; the same source builds on the PC with `make linux`).
- SSH access to the gateway.

The tool verifies every file in the package against `md5.txt` before it starts, and the module accepts only Qualcomm-signed images. Never run the tool with `-n` (skip MD5) or `-e` (erase everything).

## Constraints

- **LTE is down during the update.** From the moment the module enters download mode until it reboots (about 30 s of flashing plus module boot), `usb0` and the AT ports disappear. The connection you use to drive the update must therefore not run over the module: use Ethernet, or a WireGuard tunnel that runs over Ethernet.
- Gateways whose only uplink is LTE need either a local Ethernet connection for the duration of the update, or extra storage (expansion-card NAND or SD card) for the local-mode procedure below.
- Keep the gateway powered throughout. A power loss while images are being written can leave the module unusable.
- Quectel enforces a downgrade floor per release line. For the current line a module updated to `EC25EUXGAR08A19M1G` cannot be flashed back below `EC25EUXGAR08A13M1G_20.200.20.200`. Keep the package you are replacing archived if you may need to return to it.
- The module's USB configuration (`AT+QCFG="usbnet"`) is preserved by the update; the tests done by FoxIoT show `usb0` returning with the same RNDIS composition.

## Procedure A — relay mode (no storage needed on the gateway)

The gateway puts the module into Qualcomm emergency-download mode and relays its USB endpoint on TCP port 9008. The firmware package stays on your PC and is streamed through the gateway. The gateway firewall keeps port 9008 closed on `eth0` and on the LTE side; reach it through an SSH port-forward or over WireGuard.

1. Record the current version:

   ```sh
   cat /dev/ttyUSB2 > /tmp/at.out & CP=$!; sleep 1
   printf 'AT+QGMR\r' > /dev/ttyUSB2; sleep 2; kill $CP; cat /tmp/at.out
   ```

2. Stop the connection monitor so it cannot power-cycle the module while it is being flashed:

   ```sh
   pstree -p | grep -E 'netd|conn-monitor'   # note the PIDs of rc.netd and netd
   kill <netd-pid> <rc.netd-pid>
   ```

3. Start the relay on the gateway. Do this only when the PC side is ready; the module is without LTE from this point:

   ```sh
   QFirehose -p 9008
   ```

   Expected log lines: `interface 0 is not vendor-specific, using DIAG interface 2`, `switch to 'Emergency download mode'`, `successful, wait module reboot`, a new device `idVendor=05c6 idProduct=9008`, then `wait_client_connect`.

4. On the PC, open the channel. With SSH:

   ```sh
   ssh -N -L 9008:127.0.0.1:9008 root@<gateway>
   ```

   Over WireGuard use the gateway's tunnel address directly in the next step and skip the forward.

5. On the PC, flash:

   ```sh
   QFirehose -f <package_dir> -p 127.0.0.1:9008 -l <log_dir>
   ```

   Expected: `Totals checking 18 files md5 value, 0 file fail!`, `qtcp_connect … idVendor=05c6, idProduct=9008`, every `<program …>` answered by `ACK`, `upgrade progress 100%`, `Upgrade module successfully.` The `-l` option writes a timestamped log file; keep it as the record of the update.

6. The tool resets the module. Reboot the gateway (this also restores the connection monitor and drops any temporary firewall rule), then repeat step 1. The version string must be the new one, `ifconfig usb0` must show an address and `ping -I usb0 <host>` must work.

If the run stops before any `<program …>` line appeared, power-cycle the gateway: the module returns unchanged. If it stops after programming started, do not power-cycle; re-run step 5 against the same relay.

## Procedure B — local mode (gateway with extra storage)

Copy the package to the gateway's SD card or expansion NAND (about 64 MB unpacked; the images do not compress usefully because the system image is already a compressed filesystem), then run the tool on the gateway itself. Detach it from the SSH session, because the session may drop when the module resets:

```sh
kill <netd-pid> <rc.netd-pid>
nohup QFirehose -f /mnt/sd/<package_dir> -l /mnt/sd > /mnt/sd/update.out 2>&1 &
```

Verify as in step 6 above. The package can be copied over the LTE link beforehand, so this procedure also serves gateways whose only uplink is LTE.

## Building the tool

`base/packages/list/qfirehose/build.sh` cross-builds `QFirehose` for the gateway with the same dockcross image as the kernel build and applies FoxIoT's patch (`base/packages/list/qfirehose/0001-rndis-diag-interface.patch`). The patch makes the tool find the module's diagnostic port when the module runs in the RNDIS USB composition used by Wolf-OS, where that port is USB interface 2 rather than interface 0. Source: <https://github.com/nippynetworks/qfirehose> (Quectel's published tool). The binary is about 93 KB and depends only on the C library.

## Record keeping

For each updated gateway keep: the gateway identifier (MAC), the version before and after (`AT+QGMR`), the package name and its SHA-256, the date, and the tool's log file. FoxIoT keeps the same record for units updated at its premises.
