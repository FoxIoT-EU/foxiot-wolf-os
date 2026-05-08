# Wolf-OS Integration Security Guide

**Document:** CRA-IG-001
**Date:** 2026-05-06
**Version:** 0.2
**Status:** ACTIVE
**Regulation:** EU Cyber Resilience Act (EU) 2024/2847, Annex II point 8(f)
**Product:** Wolf Gateway — Wolf-OS configuration
**Manufacturer:** FoxIoT OÜ

---

## 1. Purpose

This guide provides the information required by CRA Annex II point 8(f) for system integrators who build applications on the Wolf-OS platform and place the combined product on the EU market.

> *Annex II, 8(f): "where the product with digital elements is intended for integration into other products with digital elements, the information necessary for the integrator to comply with the essential cybersecurity requirements set out in Annex I and the documentation requirements set out in Annex VII"*

---

## 2. When Does the Integrator Become a Manufacturer?

Per CRA Article 22(1):

> *"A natural or legal person, other than the manufacturer, the importer or the distributor, that carries out a substantial modification of a product with digital elements and makes that product available on the market, shall be considered to be a manufacturer for the purposes of this Regulation."*

**If the integrator adds their own application to Wolf-OS and places the combined product on the market, they are likely carrying out a substantial modification.** In that case, per Art. 22(2), the integrator is subject to Articles 13 and 14 for the part affected by the modification — or for the entire product if the modification impacts the cybersecurity of the product as a whole.

---

## 3. What FoxIoT Provides (Platform Scope)

FoxIoT is responsible for CRA compliance of the following Wolf-OS platform components:

| Component | FoxIoT Responsibility |
|-----------|----------------------|
| Hardware (SoC, motherboard, expansion cards, enclosure) | Design, production, CE marking |
| Custom bootloader | Development, firmware validation, failsafe mechanism |
| Linux kernel 6.12 LTS | CVE monitoring, security patches during support period |
| BusyBox-based OS | CVE monitoring, security patches during support period |
| iptables firewall | Default-deny configuration |
| WireGuard VPN support | Available for integrator to configure |
| Hardware watchdog (WDT) | Daemon with service registration API |
| Dual firmware failsafe boot | Automatic fallback mechanism |
| FoxIoT build system | Firmware image creation tools |
| Security updates for platform | Free during 5-year support period |
| Platform SBOM | Components within FoxIoT scope |
| Platform vulnerability handling | Per CRA-VH-001 |
| Platform technical documentation | CRA-TD-001 |

**FoxIoT does NOT cover:**
- Integrator's application layer
- Integrator's configuration choices
- Integrator's deployment environment
- Any software components added by the integrator

---

## 4. Integrator's CRA Obligations

If the integrator is considered a manufacturer (Art. 22), they must comply with the following. Each item references the specific CRA article or annex.

### 4.1 Essential Cybersecurity Requirements (Annex I, Part I)

| Requirement | What the Integrator Must Do |
|------------|---------------------------|
| 2(a) No known exploitable vulnerabilities | Audit their own application components for CVEs before market placement |
| 2(b) Secure by default | Ensure their application has secure default settings; do not open unnecessary ports |
| 2(c) Security update mechanism | Provide a way to update their application layer independently or as part of the full firmware |
| 2(d) Protection from unauthorised access | Implement authentication for any interfaces their application exposes |
| 2(e) Confidentiality | Use TLS/SSL for any network communication their application performs |
| 2(f) Integrity | Include their application in the FIT firmware image to benefit from bootloader validation |
| 2(g) Data minimisation | Only collect and store data necessary for their application's purpose |
| 2(h) Availability | Register their application with the hardware watchdog (see section 5.1) |
| 2(i) Minimise impact on other devices | Ensure their application does not degrade the security of connected devices |
| 2(j) Limit attack surfaces | Only expose necessary network ports; maintain the default-deny firewall policy |
| 2(k) Reduce incident impact | Implement error handling that does not compromise device security |
| 2(l) Security logging | Log security-relevant events from their application |
| 2(m) Data removal | Provide a mechanism (factory reset, decommission flow, or equivalent) that allows users to permanently remove all data and settings, including the integrator's application data, wherever it is stored |

### 4.2 Vulnerability Handling (Annex I, Part II)

| Requirement | What the Integrator Must Do |
|------------|---------------------------|
| (1) SBOM | Generate an SBOM covering their application components (in commonly used, machine-readable format) |
| (2) Address vulnerabilities | Establish a process to patch vulnerabilities in their application without delay |
| (3) Regular security testing | Test their application for security issues regularly |
| (4) Public disclosure | Publish information about fixed vulnerabilities after patches are available |
| (5) CVD policy | Establish a coordinated vulnerability disclosure policy for their application |
| (6) Security contact | Provide a contact address for reporting vulnerabilities in their application |
| (7) Secure update distribution | Distribute their application updates securely |
| (8) Free security updates | Provide security updates free of charge during the support period |

### 4.3 Documentation (Annex VII)

The integrator must prepare their own technical documentation for the parts they are responsible for. This includes:

| Annex VII Section | What the Integrator Must Provide |
|-------------------|--------------------------------|
| 1 — General description | Description of their application, its purpose, and software versions |
| 2(a) — Design info | Architecture of their application, how it integrates with Wolf-OS |
| 2(b) — Vulnerability handling | Their own SBOM, CVD policy, security contact, update mechanism |
| 2(c) — Production | How they build, test, and distribute the combined product |
| 3 — Risk assessment | Cybersecurity risk assessment for their application layer (per Art. 13(2-3)) |
| 4 — Support period | Their support period rationale |
| 5 — Standards | Harmonised standards or solutions adopted |
| 6 — Test reports | Security test results for the combined product |
| 7 — EU DoC | Their own EU Declaration of Conformity (if they are the manufacturer of the combined product) |

### 4.4 Reporting (Art. 14)

If the integrator discovers an actively exploited vulnerability or severe incident in their application:
- **24 hours**: Early warning to CSIRT + ENISA
- **72 hours**: Detailed notification
- **14 days** (vulnerability) / **1 month** (incident): Final report

---

## 5. Technical Integration Requirements

### 5.1 Hardware Watchdog Registration

**CRA tie-in:** Annex I, Part I(2)(h) — availability and resilience.

**What the integrator must do:** register the application as a client of the Wolf-OS watchdog daemon (`watchdogd`) so that an application crash or hang triggers automatic system reset.

**Why it matters for CRA:** an application that hangs without registering with the watchdog leaves the device in an undefined state. Registering ensures the platform-level resilience guarantee extends to the integrator's application.

**Where to find the API and reference code:** [Wolf-OS public repository — `docs/watchdog.md`](https://github.com/FoxIoT-EU/foxiot-wolf-os/blob/main/docs/watchdog.md). Includes the protocol description, a shell-script pattern, and a C reference implementation.

### 5.2 Firewall Rules

**CRA tie-in:** Annex I, Part I(2)(b) secure by default + Part I(2)(j) limit attack surfaces.

**What the integrator must do:**
- Open only the ports the application strictly needs.
- Maintain the default-deny policy. Do **not** flush or replace existing rules — only append new ones.
- Document every additional open port in the integrator's own user information (Annex II).

**Where to apply changes:** the firewall is configured by the script `/etc/firewall` in the integrator's distro folder of the Wolf-OS build tree. Wolf-OS ships this script with default-deny `INPUT`/`FORWARD`, `ACCEPT` for `lo`, `wg0`, established connections, ICMP, and SSH on `eth0`. Add application-specific rules at the end of the script.

**Where to find the reference firewall script:** [Wolf-OS public repository — `distro/example/root/etc/firewall`](https://github.com/FoxIoT-EU/foxiot-wolf-os/blob/main/distro/example/root/etc/firewall).

### 5.3 Filesystem Usage

**CRA tie-in:** Annex I, Part I(2)(f) integrity of stored data + Part I(2)(m) secure data removal.

**What the integrator must do (principles):**
- **Do not write to the read-only root filesystem at runtime.** System binaries and signed firmware content must remain immutable; modifying them defeats the bootloader integrity check.
- **Persistent application data goes on a writable partition.** Use writable persistent storage (NAND or SD) for any state that must survive reboots — configuration, application data, security-relevant logs.
- **Decommissioning must wipe sensitive data.** If the integrator's product offers a factory-reset / decommission flow, it must erase all integrator application data wherever it lives (writable NAND, SD card, anywhere else the integrator stores it). This is the integrator's responsibility for any storage they introduce.

**FoxIoT reference layout** (example only — integrators may use different mount points and partitioning, as long as the principles above are met):

| Mount | Runtime access | Used for |
|-------|---------------|----------|
| `/` | Read-only | System binaries |
| `/mnt/rodata` | UBIFS on NAND, **mounted read-only at runtime** but remountable read-write by privileged processes for explicit config updates | Wolf-OS reference: WireGuard keys, netd configuration, integrator's persistent `rc.local`. Suitable for sensitive credentials and configuration that should not be modified by routine application activity. |
| `/mnt/rwdata` | UBIFS on NAND, read-write persistent | Wolf-OS reference: application configuration, persistent runtime state, audit log at `/mnt/rwdata/log/auth.log` |
| `/mnt/sdcard` | ext4 on optional SD, read-write persistent | Wolf-OS reference: bulk data, extended logs |
| RAM overlay | Read-write, volatile | Temporary files (lost on reboot) |

The two-tier writable layout (`rodata` mounted read-only by default, `rwdata` mounted read-write) is itself a defence-in-depth pattern: storing sensitive credentials in a partition that requires an explicit admin remount to write reduces the attack surface for runtime processes. Integrators are encouraged to use this pattern (or an equivalent) for VPN keys, TLS certificates, signing keys, and other long-lived secrets. The example distro in the public Wolf-OS repository uses exactly this layout; integrators are free to add, rename, or restructure mount points to suit their product, provided the read-only-by-default / writable / decommission-wipe principles are satisfied.

### 5.4 Firmware Image Building

**CRA tie-in:** Annex I, Part I(2)(f) integrity of software + Part I(2)(c) secure update mechanism.

**What the integrator must do:** build the combined firmware (Wolf-OS + integrator application) using the FoxIoT build system, so that the resulting FIT image is signed and validated as a single unit.

**Why it matters for CRA:**
- The bootloader validates the FIT image structure and component checksums on each boot.
- The Ed25519 platform signature covers the entire firmware, including the integrator's application binaries — no separate signing step is needed for the application.
- The dual-firmware failsafe and update mechanism then apply to the combined product as a whole.

**Where to find the build instructions:** [Wolf-OS public repository — `docs/developer-quick-start.md` (§ "Adding Your Application to the Firmware")](https://github.com/FoxIoT-EU/foxiot-wolf-os/blob/main/docs/developer-quick-start.md). Covers distro folder layout, including binaries via `rootfs.list`, adding startup scripts, and hooking into `rc.local`.

### 5.5 Network Communication

**CRA tie-in:** Annex I, Part I(2)(e) confidentiality of data in transit + Part I(2)(f) integrity of commands and data.

**What Wolf-OS provides for network security:**

- **OpenSSL** libraries — TLS available to any protocol the integrator builds on top.
- **Dropbear SSH** with **key-only authentication** enabled by default on port 22.
- **WireGuard** VPN — kernel module, `wg` tools, configuration hook at `/etc/rc.wireguard`.
- **RS-485** hardware with Linux serial drivers (the physical layer for Modbus RTU and similar industrial fieldbuses).
- **Default-deny iptables firewall** (covered in §5.2).

**What the integrator must do:**

- For any network protocol the integrator implements or includes (MQTT, HTTP, custom application protocols): use TLS, authenticate both ends, do not run cleartext on untrusted networks. Where TLS is impractical, route the protocol through WireGuard.
- Do not expose additional unencrypted management interfaces alongside SSH.
- Do not enable SSH password authentication.
- For protocols with no built-in encryption or authentication (Modbus RTU, Modbus TCP, plain industrial fieldbuses): document the limitation in the integrator's user information per Annex II, and require network segmentation (isolated OT network, not exposed to the internet) as a deployment condition.

### 5.6 Authentication

If the integrator's application provides user-facing interfaces (web UI, API, CLI):
- Implement authentication appropriate to the deployment risk (e.g., a strong password policy with sufficient length and complexity, second factor for remote access, account lockout / rate limiting on failed attempts).
- Do not store passwords in cleartext. Use a salted, computationally expensive password-hashing algorithm — **PBKDF2** (with HMAC-SHA-256 or HMAC-SHA-512, sufficient iterations) is the most widely accepted choice; alternatives of equal or greater strength are acceptable depending on the integrator's target market.
- Default-credential policy (CRA Annex I, Part I(2)(d)): default passwords must EITHER be unique per individual product OR be required to be changed before initial use. Either approach is CRA-compliant; the integrator chooses. (For reference, FoxIoT's own Wolf-App ships with a documented shared default `foxiot`/`foxiot` paired with a forced-change-on-first-login modal that blocks all UI actions until a new password is set — the second arm of the regulation.)
- Log authentication events (successful and failed attempts) per §5.7.

### 5.7 Logging

**CRA tie-in:** Annex I, Part I(2)(l) — recording and monitoring of security-relevant internal activity.

**What the integrator must do:** record security-relevant events from the integrator's application. At minimum:
- Authentication attempts (success/failure, timestamp, source)
- Configuration changes
- Firmware updates
- Error conditions that may indicate a security issue

The records must be persistent (survive reboot) so they remain useful for incident investigation. Storage location, format, retention, and whether to forward logs anywhere are integrator decisions.

---

## 6. Support Period Coordination

| Aspect | FoxIoT (platform) | Integrator (application) |
|--------|-------------------|-------------------------|
| Support period start | Date FoxIoT places Wolf-OS on market | Date integrator places combined product on market |
| Support period duration | 5 years | Determined by integrator (minimum 5 years per CRA Art. 13(8)) |
| Security updates | Platform updates (kernel, OS, bootloader) | Application updates |
| CVE monitoring | Platform components | Application components |
| CSIRT/ENISA reporting | Platform vulnerabilities | Application vulnerabilities |

**Important:** The integrator's support period for the combined product may extend beyond FoxIoT's platform support period. The integrator should plan for this scenario (e.g., by maintaining their own kernel patches or planning product end-of-life).

### 6.1 Communicating the Support Period End Date

**CRA tie-in:** Art. 13(19) — the security-update end date must be accessible to the end user at the time of purchase, including at least month and year.

**What FoxIoT provides for the platform:**
- A public 5-year support commitment from date of placement on market.
- A per-device end date computed as `manufacture_date + 5 years + 4 months` (the 4-month buffer covers shelf time so the 5-year market-placement obligation is always met). The integrator can derive this themselves from the FoxIoT invoice date (placement on market) — `invoice_date + 5 years` — or from their own delivery records. If the integrator wants the per-device manufacture-date-based figure, that is recorded against each device's MAC in FoxIoT's CRA conformity log and is available on request to security@foxiot.eu.

**What the integrator must do:**
- Communicate the integrator's own support-period end date (the integrator is the manufacturer of the combined product under Art. 22, so the obligation is theirs).
- If the integrator's support period extends past FoxIoT's platform support period, communicate clearly to the end user which components are still under platform support and which are integrator-supported only.
- Choose a communication channel suitable for the integrator's product (label, UI, packaging, web URL — Art. 13(19) allows any of these).

---

## 7. Checklist for Integrators

Before placing the combined product (Wolf-OS + application) on the EU market:

| # | Item | CRA Reference |
|---|------|---------------|
| 1 | Application components audited for known CVEs | Annex I, Part I(2)(a) |
| 2 | Application has secure default configuration | Annex I, Part I(2)(b) |
| 3 | Application update mechanism works | Annex I, Part I(2)(c) |
| 4 | Authentication implemented for all interfaces | Annex I, Part I(2)(d) |
| 5 | All network communication uses encryption | Annex I, Part I(2)(e) |
| 6 | Application included in FIT firmware image | Annex I, Part I(2)(f) |
| 7 | Application registered with hardware watchdog | Annex I, Part I(2)(h) |
| 8 | Only necessary ports opened in firewall | Annex I, Part I(2)(j) |
| 9 | Security events logged | Annex I, Part I(2)(l) |
| 10 | Data-removal mechanism implemented (factory reset, decommission, or equivalent) that wipes all application data | Annex I, Part I(2)(m) |
| 11 | Application SBOM generated | Annex I, Part II(1) |
| 12 | Vulnerability handling process established | Annex I, Part II(2-6) |
| 13 | Security contact published | Annex I, Part II(6) |
| 14 | CVD policy published | Annex I, Part II(5) |
| 15 | Technical documentation prepared (Annex VII) | Art. 31 |
| 16 | Risk assessment completed | Art. 13(2-3) |
| 17 | User information prepared (Annex II) | Art. 13(18) |
| 18 | EU Declaration of Conformity prepared | Art. 28 |
| 19 | CE marking affixed | Art. 30 |
| 20 | Support period defined and communicated | Art. 13(8), 13(19) |
| 21 | CSIRT/ENISA reporting process ready | Art. 14 |

---

## Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 0.1 | 2026-03-05 | - | Initial integration guide per Annex II point 8(f) |
| 0.2 | 2026-05-06 | - | Filled in §5.1–5.4 with concrete "what to do / what not to do" content, each section linking to public Wolf-OS docs (`watchdog.md`, `firewall` script, `developer-quick-start.md`) for the implementation detail. §6.1 rewritten — removed EEPROM/5.5-year wording; replaced with CRA-SP-001 lookup approach (`manufacture_date + 5y + 4mo`, looked up against per-device conformity log). Document now reads as an integrator's CRA "do/don't" guide rather than a developer reference. |
