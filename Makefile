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


CFLAGS := \
		-O0 \
		-std=gnu99 \
		-Wall \
		-static \
		-nostdlib \
		-nodefaultlibs \
		-mgeneral-regs-only \
		-fno-common \
		-fno-strict-aliasing \
		-fPIC \
		-fno-builtin

TZ_LOG_PAGES := 0x40300000

EXTRA_CFLAGS=-DTZ_LOG_PAGES=$(TZ_LOG_PAGES)

all: drv_objs baremetal.o stubs.o

clean:
	rm -f *.o stubs.c drv.a

baremetal.o: baremetal.c
	$(CROSS_COMPILE)gcc $(CFLAGS) $(LINUXINCLUDE) -c -o $@ $^

stubs.o: stubs.c
	$(CROSS_COMPILE)gcc $(CFLAGS) $(LINUXINCLUDE) -c -o $@ $^

stubs.c: drv_objs gen_stubs.py
	cd $(srctree)/drivers/misc/mediatek/teei/400/; rm -f drv.o drv.a; \
	$(CROSS_COMPILE)ld.gold -r -o drv.o $(TEEI_OBJS) ; \
	$(CROSS_COMPILE)ar -rcs -o drv.a $(TEEI_OBJS) ; \
	$(CROSS_COMPILE)nm -u drv.o > undef.txt
	./gen_stubs.py < $(srctree)/drivers/misc/mediatek/teei/400/undef.txt > stubs.c
	cp $(srctree)/drivers/misc/mediatek/teei/400/drv.a .

drv_objs: 
	cd $(srctree); make -j 8 EXTRA_CFLAGS=$(EXTRA_CFLAGS) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) drivers/misc/mediatek/teei/400/
