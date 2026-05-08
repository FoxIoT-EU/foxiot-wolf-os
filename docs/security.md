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
