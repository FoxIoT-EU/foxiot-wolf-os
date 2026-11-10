VERSION = 0.1.3
# Dev kit only: shared first-login password. The partner's own firmware
# (built without PASSWORD) removes password login. See docs/security.md.
PASSWORD = "foxiot"
IMAGE_PREFIX = root_devkit_$(VERSION)
MODEL = wolf
INITRD_LIST = rootfs.list
ROOTFS_LIST = initrd.list
IMAGE_ITS ?= image.its
DRAM ?= NUC980DF71YC.ini
DEVICETREE_DIR_PATH=devicetrees
FW_KEY = ../../fw_keys/example/secret.key

include ../../base/Makefile_dist.inc
