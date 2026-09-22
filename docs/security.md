# Security

> For the full set of CRA integrator obligations (Annex II point 8(f)), see [CRA Integration Guide](cra-integration-guide.md).

## Authentication

### Default: SSH Public Key Only

Wolf-OS ships with password authentication **disabled** by default. Access to the controller requires an SSH public key deployed during the firmware build.

This default exists to comply with the **EU Cyber Resilience Act (CRA)**:

| CRA Reference | Requirement | How Wolf-OS Complies |
|---|---|---|
| Annex I, Part I(1)(d) | Products must not have default passwords common across multiple devices | Password auth disabled by default; no shared password |
| Annex I, Part I(2)(c) | Appropriate access control mechanisms must be in place | SSH public key authentication per device |
| Annex I, Part I(1)(c) | Products must be secure by default | Key-only SSH out of the box |

### Enabling Password Authentication

Password authentication can be enabled for development or testing by setting `PASSWORD` in your distro Makefile:

```make
PASSWORD = "your_unique_password"
```

See [Developer Quick Start - Security Settings](developer-quick-start.md#default-root-password-and-security-settings) for details.

> ⚠️ **Do not ship devices with password authentication enabled.** Each device would share the same password with no mechanism to change it at runtime.

## Audit Logging

### Auth Event Logging

Wolf-OS logs all authentication events via syslog. The `auth.*` facility is captured and persisted across reboots.

- During boot, syslog writes auth events to `/var/log/syslog/auth.log`
- After storage is mounted, logs are persisted to `/mnt/rwdata/log/auth.log`

To view auth logs on a running controller:

```bash
cat /mnt/rwdata/log/auth.log
```

### Identifying SSH Users

The `ssh-users` utility lists SSH key fingerprints and their owners from `/root/.ssh/authorized_keys`.

```bash
# List all keys with fingerprints
ssh-users

# Find which key matches a fingerprint (partial match)
ssh-users <fingerprint>
```

This is useful for identifying which key was used for a login by cross-referencing with Dropbear's auth log entries.

### Log Retention

The authentication log is written by `syslogd` to `/var/log/syslog/auth.log`. At boot, `rc.local` moves this log to the persistent data partition (`/mnt/rwdata/log/auth.log`) and symlinks `/var/log/syslog` to it, so entries survive reboots and power cycles.

The log is size-rotated by `syslogd -s 250 -b 1`: the current file grows to 250 KB, then it is renamed `auth.log.0` and a new file is started. One rotated file is kept. At the typical line length of about 100 bytes this guarantees that **at least 2 300 of the most recent authentication events, and always the newest one**, are retained on the device at any time; in practice the two files together hold roughly twice that.

Every entry carries a timestamp from the system clock. The NUC980 real-time clock keeps time across resets while the device is powered; after boot, `chrony` synchronises the clock with NTP. Entries written before the first NTP synchronisation carry the real-time-clock value (or the epoch default if the clock was never set) and remain in order in the file.

To forward the log to an external server instead, or in addition, point `syslogd -R <host>` at your log collector in your distro's `rc.local`.

## Data Erasure and Decommissioning

Wolf-OS itself stores only a small amount of data that can identify people: the authentication log (source IP addresses, key fingerprints) and the comments in `authorized_keys`. Your application may store more. Before a controller is disposed of, returned, or transferred to another owner, erase the persistent partitions as root over SSH:

```bash
# 1. Authentication log and everything else on the read-write data partition
rm -rf /mnt/rwdata/log /mnt/rwdata/app
# (or erase the whole partition; adapt the paths if your application keeps data elsewhere)

# 2. Keys and configuration on the read-only data partition
mount -o remount,rw /mnt/rodata
rm -rf /mnt/rodata/.ssh /mnt/rodata/wireguard /mnt/rodata/netd
mount -o remount,ro /mnt/rodata

# 3. Remove the SD card, if one is fitted (application and watchdog logs may be on it)

# 4. Reboot: new SSH host keys are generated automatically
reboot
```

Enrolled administrator keys live inside the firmware image; to remove them, install a firmware image built without those keys (see [Firmware Signing](firmware-signing.md)) or leave the device without a network connection.

The commands above erase data at the filesystem level. NAND flash uses wear levelling, so residual traces may remain recoverable with specialised equipment. For high-sensitivity deployments, physically destroy the flash memory or the whole controller at end of life.

Only the root user can trigger erasure, so it cannot be started accidentally or by an unauthorised person; run it deliberately as the last step before the device leaves your control.
