# Security

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
