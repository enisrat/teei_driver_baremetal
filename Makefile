CROSS_COMPILE=/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-

KERNEL_SRCDIR_WITH_TEEI ?= ./android_kernel_with_teei400
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

TEEI_OBJS := tz_driver/backward_driver.o tz_driver/irq_register.o tz_driver/notify_queue.o tz_driver/teei_cancel_cmd.o tz_driver/teei_smc_call.o tz_driver/fdrv.o tz_driver/switch_queue.o tz_driver/teei_client_main.o tz_driver/teei_task_link.o 
TEEI_OBJS += tee/soter/call.o tee/soter/core.o


baremetal.o: baremetal.c
	$(CROSS_COMPILE)gcc $(LINUXINCLUDE) -c -o $@ $^

stubs.o: stubs.c
	$(CROSS_COMPILE)gcc $(LINUXINCLUDE) -c -o $@ $^

stubs.c: drv_objs
	cd $(srctree)/drivers/misc/mediatek/teei/400/; rm temp.o; \
	$(CROSS_COMPILE)ld.gold -r -o temp.o $(TEEI_OBJS) ; \
	$(CROSS_COMPILE)nm -u temp.o > undef.txt
	./gen_stubs.py < $(srctree)/drivers/misc/mediatek/teei/400/undef.txt > stubs.c

drv_objs: 
	cd $(srctree); make ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) drivers/misc/mediatek/teei/400/
