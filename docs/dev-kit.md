# Dev Kit: First Steps

This guide is for development partners who received a FoxIoT Wolf controller as a **dev kit**. It takes you from the first login to running your own firmware.

### Quick Links
- [1. What Is on Your Controller](#1-what-is-on-your-controller)
- [2. Connect and Log In](#2-connect-and-log-in)
- [3. Create Your Keys](#3-create-your-keys)
- [4. Build and Install Your First Firmware](#4-build-and-install-your-first-firmware)

---

## 1. What Is on Your Controller

Your controller runs plain Wolf OS, built from this repository (`distro/devkit`):

- No FoxIoT application software
- Optional LTE modem, off by default ([how to enable](developer-quick-start.md#configuring-netd-network-daemon))
- Ethernet gets its address via DHCP
- Root login with the password **`foxiot`**

The password is for your **first login only**, so you can install your own firmware. Your own firmware has password login switched off and uses your SSH key instead.

> 💡 The dev kit accepts firmware signed with **any** key, so your first own build installs without FoxIoT's involvement. Your firmware then contains your own public key, and from that point the controller only accepts firmware signed by you. See [Firmware Signing](firmware-signing.md).

---

## 2. Connect and Log In

1. Connect the controller to your network with an Ethernet cable and power it on.
2. Find its IP address, for example in your router's DHCP client list. The hostname is `wolf-<MAC address>`. More ways: [Finding the Controller's IP Address](developer-quick-start.md#finding-the-controllers-ip-address).
3. Log in with password `foxiot`:
   ```bash
   ssh root@YOUR_CONTROLLER_IP
   ```

> 💡 No DHCP server? Hold the button while the controller boots and release it within 5 seconds (the yellow LED blinks). The controller then starts with the fixed address `192.168.2.127`. See [Recovery Mode](developer-quick-start.md#recovery-mode).

---

## 3. Create Your Keys

You need two keys on your computer:

| Key | Used for | How to create it |
|-----|----------|------------------|
| SSH key | Logging in to the controller | [Getting Started - Add Your SSH Key](getting-started.md#5-add-your-ssh-key) |
| Firmware signing key | Signing your firmware images | [Getting Started - Set Up Firmware Signing](getting-started.md#3-set-up-firmware-signing) |

> ⚠️ **Keep your firmware secret key (`fw_keys/YOUR_PROJECT_NAME/secret.key`) safe and backed up.** After your first install, the controller only accepts firmware signed with it. If you lose it, you can no longer update the controller yourself.

---

## 4. Build and Install Your First Firmware

Follow the [Getting Started Guide](getting-started.md) steps 1–8: create your project, add both keys, build the image, copy it to the controller and run `install`.

> ⚠️ **Check that your SSH public key is in your firmware before you install it.**
>
> Your firmware has password login switched off. If your SSH key is missing, you can no longer log in after the reboot. The controller can still be re-flashed over its micro USB port, but that needs a special tool, so contact **support@foxiot.eu** if it happens.
>
> Before installing, check that this file contains your public key:
> ```bash
> cat distro/YOUR_PROJECT_NAME/root/etc/dropbear/authorized_keys
> ```
> While developing, you can also keep password login in your own builds, see [Default Root Password and Security Settings](developer-quick-start.md#default-root-password-and-security-settings). Remove it before your product goes to customers.

After `install`, reboot the controller:

```bash
reboot
```

Your firmware is now running. Log in with your SSH key:

```bash
ssh root@YOUR_CONTROLLER_IP
```

From here on, the [Developer Quick Start](developer-quick-start.md) covers everything else: adding your application, network and LTE configuration, and the I/O pins.

---

## Support

For questions about the dev kit, contact **support@foxiot.eu**.
