CROSS_COMPILE ?= /opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-

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
TEEI_OBJS += tee/soter/call.o tee/soter/core.o tee/tee_shm.o


CFLAGS := \
		-g \
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
		-fno-builtin \
		-D__KERNEL__

TZ_LOG_PAGES := 0x40300000

EXTRA_CFLAGS=-DTZ_LOG_PAGES=$(TZ_LOG_PAGES)

all: drv_objs baremetal.o stubs.o vfs.o km.o test_ta_c1.o

clean:
	rm -f *.o stubs.c drv.a

test_ta_c1.o: test_ta_c1.c
	$(CROSS_COMPILE)gcc $(CFLAGS) -I$(srctree)/drivers/misc/mediatek/teei/400/common/include -I$(srctree)/drivers/misc/mediatek/teei/400/tee/soter $(LINUXINCLUDE) -c -o $@ $^


km.o: km.c 
	$(CROSS_COMPILE)gcc $(CFLAGS) -I$(srctree)/drivers/misc/mediatek/teei/400/common/include -I$(srctree)/drivers/misc/mediatek/teei/400/tee/soter $(LINUXINCLUDE) -c -o $@ $^

vfs.o: vfs.c thh/ta/c09c9c5daa504b78b0e46eda61556c3a.h thh/ta/8b22aba81ef0ccbfd9f5f4b634127e15.h thh/ta/d91f322ad5a441d5955110eda3272fc0.h thh/ta/0102030405060708090a0b0c0d0e0f10.h thh/ta/c1882f2d885e4e13a8c8e2622461b2fa.h thh/ta/93feffccd8ca11e796c7c7a21acb4932.h thh/ta/rpmb.h
	$(CROSS_COMPILE)gcc $(CFLAGS) $(LINUXINCLUDE) -c -o $@ vfs.c

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
	cd $(srctree); make DEBUG_FLAGS=-g -j8 EXTRA_CFLAGS=$(EXTRA_CFLAGS) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) drivers/misc/mediatek/teei/400/

thh/ta/%.h: thh/ta/%.ta
	xxd -i $^ > $@

thh/ta/rpmb.h: thh/ta/rpmb
	xxd -i $^ > $@