PACKAGE_INCLUDE_PATH ?= ../../base/packages/cpio
PACKAGE_SEARCH_PATH ?= ../../base/packages/list
GEN_INIT_FS ?= ../../base/bin/gen_init_fs
ALIGN_FS ?= ../../base/bin/align_fs
KERNEL ?= ../../base/kernel/zImage
DEVICETREE_DIR_PATH ?= .
IMAGE_ITS ?= ../../base/image.its
BUILD ?= .build
IMAGE_PREFIX ?= image
USB_PATH ?= ""
DRAM ?= NUC980DF61YC.ini
TYPE ?= spinand

ITS_FIT_DESCRIPTION ?= default
ITS_IMAGE_KERNEL_DESCRIPTION ?= linux
ITS_IMAGE_FDT_DESCRIPTION ?= devicetree
ITS_IMAGE_RAMDISK_DESCRIPTION ?= ramdisk
ITS_IMAGE_FILESYSTEM_DESCRIPTION ?= filesystem
ITS_CONFIGURATION_DESCRIPTION ?= default

all: fit-image

kernel:
	if [ ! -d $(BUILD) ]; then mkdir $(BUILD); fi
	cp $(KERNEL) $(BUILD)/kernel.bin

devicetree:
	if [ ! -d $(BUILD) ]; then mkdir $(BUILD); fi

	for f_path in $(DEVICETREE_DIR_PATH)/*.dts; do \
		f_name="$${f_path##*/}"; \
		cpp -nostdinc -x assembler-with-cpp -I../../base/devicetree -undef -D__DTS__ $${f_path} > $(BUILD)/$${f_name}; \
		dtc -O dtb -idevicetree -W no-unit_address_vs_reg -o $(BUILD)/$${f_name%.dts}.dtb < $(BUILD)/$${f_name}; \
	done	
	
ifdef VERSION
version:
	/bin/echo -ne "$(VERSION)" > root/VERSION
else
version:
	true
endif

ifdef PASSWORD
passwd:
	/bin/echo "root:`mkpasswd $(PASSWORD)`:0:0:root:/root:/bin/sh" >root/etc/passwd
else
passwd:
	/bin/echo "root:x:0:0:root:/root:/bin/sh" >root/etc/passwd
endif

initrd:
	if [ ! -d $(BUILD) ]; then mkdir $(BUILD); fi
	$(GEN_INIT_FS) -I$(PACKAGE_INCLUDE_PATH) -S$(PACKAGE_SEARCH_PATH) -c $(INITRD_LIST) >$(BUILD)/initrd.cpio
	lzmp -z -c $(BUILD)/initrd.cpio > $(BUILD)/initrd.cpio.lzma

rootfs:
	if [ ! -d $(BUILD) ]; then mkdir $(BUILD); fi
	rm -rf $(BUILD)/rootfs
	$(GEN_INIT_FS) -I$(PACKAGE_INCLUDE_PATH) -S$(PACKAGE_SEARCH_PATH) $(ROOTFS_LIST) -j $(BUILD)/rootfs
	/usr/sbin/mkfs.jffs2 --squash --pad --no-cleanmarkers --eraseblock=128KiB -D $(BUILD)/rootfs/list.txt -d $(BUILD)/rootfs/root/ -o $(BUILD)/rootfs_nosum.jffs2
	/usr/sbin/sumtool --pad -e 128KiB -i $(BUILD)/rootfs_nosum.jffs2 -o $(BUILD)/rootfs.jffs2
	dd if=/dev/urandom of=$(BUILD)/rootfs.jffs2 conv=notrunc oflag=append bs=128 count=1024

fit-image:
	make kernel
	make devicetree
	make passwd
	make version
	make initrd
	make rootfs
	if [ ! -d $(BUILD) ]; then mkdir $(BUILD); fi
	cpp -nostdinc -x assembler-with-cpp -DITS_FIT_DESCRIPTION="\"$(ITS_FIT_DESCRIPTION)\"" -DITS_IMAGE_KERNEL_DESCRIPTION="\"$(ITS_IMAGE_KERNEL_DESCRIPTION)\"" -DITS_IMAGE_FDT_DESCRIPTION="\"$(ITS_IMAGE_FDT_DESCRIPTION)\"" -DITS_IMAGE_RAMDISK_DESCRIPTION="\"$(ITS_IMAGE_RAMDISK_DESCRIPTION)\"" -DITS_IMAGE_FILESYSTEM_DESCRIPTION="\"$(ITS_IMAGE_FILESYSTEM_DESCRIPTION)\"" -DITS_CONFIGURATION_DESCRIPTION="\"$(ITS_CONFIGURATION_DESCRIPTION)\"" $(IMAGE_ITS) > $(BUILD)/fit-image.its
	
	mkimage -D "-W no-unit_address_vs_reg" -f $(BUILD)/fit-image.its $(IMAGE_PREFIX).itb
	$(ALIGN_FS) $(IMAGE_PREFIX).itb
	chmod 755 $(IMAGE_PREFIX).itb
	
package:
	INITRD_SOURCE_PATH=$(INITRD_SOURCE_PATH)/root $(GEN_INIT_FS) -I$(INITRD_SOURCE_PATH) -c package.list >package.cpio

clean:
	rm -rf $(BUILD) *.itb

install:
	nucutil -u $(USB_PATH) -d $(DRAM) -t $(TYPE) -c nandfix -f Makefile -a 0x1040000 -c write -f $(IMAGE_PREFIX).itb -a 0x40000 -c write

info:
	mkimage -l $(IMAGE_PREFIX).itb

