# Getting Started Guide

This guide explains how to quickly create your own firmware image for the FoxIoT Wolf controllers and install it.

---

## 1. Clone the Repository

First, clone the `foxiot-wolf-os` repository and enter the folder:
```bash
git clone https://github.com/FoxIoT-EU/foxiot-wolf-os.git
cd foxiot-wolf-os
```

---

## 2. Create Your Project

Copy the example project to a new folder:
```bash
cp -r distro/example/ distro/YOUR_PROJECT_NAME
```
Replace `YOUR_PROJECT_NAME` with your own project name (for example, your company name).

---

## 3. Set the Project Prefix

Edit the `Makefile` inside your new project directory and change the `IMAGE_PREFIX` to match your project name:
```bash
vim distro/YOUR_PROJECT_NAME/Makefile
```
Example:
```
IMAGE_PREFIX = your_project
```

---

## 4. Build the Image

Build your firmware image using Docker:
```bash
./build-in-docker.sh YOUR_PROJECT_NAME
```

The output `.itb` file will appear inside the `build/` directory.

> ℹ️ **Tip:** Make sure Docker is installed and running on your system.  
> For more detailed information, see [Building the OS](building.md).

---

## 5. Copy the Image to the Controller

After building, copy the `.itb` image file to your controller:
```bash
scp -O build/root_YOUR_PROJECT_NAME_0.1.1.itb root@YOUR_CONTROLLER_IP:/tmp/
```
Replace `YOUR_CONTROLLER_IP` with your controller’s IP address.

---

## 6. Install the New Image

Log in to the controller via SSH:
```bash
ssh root@YOUR_CONTROLLER_IP
```
Then install the new firmware:
```bash
install /tmp/root_YOUR_PROJECT_NAME_0.1.1.itb
```

---

## 7. Reboot the Controller

Finally, reboot the controller to boot into the new system:
```bash
reboot
```

---

# Notes

- Replace `YOUR_PROJECT_NAME` consistently everywhere with your actual project name.
- Replace `YOUR_CONTROLLER_IP` with the real IP address of your controller.
- Make sure you have SSH access to the controller (default user: `root`).

---

✅ Now your controller will be running the new firmware built from your own project!

