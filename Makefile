CROSS_COMPILE=/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-

srctree := $(KERNEL_SRCDIR_WITH_TEEI)
objtree := $(srctree)
hdr-arch := arm64

KBUILD_SRC := false

# Use USERINCLUDE when you must reference the UAPI directories only.
USERINCLUDE    := \
		-I$(srctree)/arch/$(hdr-arch)/include/uapi \
		-I$(objtree)/arch/$(hdr-arch)/include/generated/uapi \
		-I$(srctree)/include/uapi \
		-I$(objtree)/include/generated/uapi \
                -include $(srctree)/include/linux/kconfig.h

# Use LINUXINCLUDE when you must reference the include/ directory.
# Needed to be compatible with the O= option
LINUXINCLUDE    := \
		-I$(srctree)/arch/$(hdr-arch)/include \
		-I$(objtree)/arch/$(hdr-arch)/include/generated \
		$(if $(KBUILD_SRC), -I$(srctree)/include) \
		-I$(srctree)/drivers/misc/mediatek/include \
		-I$(objtree)/include \
		$(USERINCLUDE)



gen_stubs: drv_objs
	cd $(KERNEL_SRCDIR_WITH_TEEI)/drivers/misc/mediatek/teei/400/; rm built-in.o teei.o temp.o; \
	$(CROSS_COMPILE)ld.gold -r -o temp.o *.o ; \
	$(CROSS_COMPILE)nm -u temp.o > undef.txt
	./gen_stubs.py < $(KERNEL_SRCDIR_WITH_TEEI)/drivers/misc/mediatek/teei/400/undef.txt > stubs.c

drv_objs: 
	cd $(KERNEL_SRCDIR_WITH_TEEI); make ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) drivers/misc/mediatek/teei/400/
