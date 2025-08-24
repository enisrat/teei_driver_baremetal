- Kernel from [github.com/xiaomi-mt6785-dev/android_kernel_xiaomi_mt6785]()

```
make ARCH=arm64 CROSS_COMPILE=/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu- menuconfig

Device Drivers -> Security -> ... TEEI
```

# tee/soter

- C API to "SOTER"
  - Should be usable as is
- Other stuff in tee/ only for user?
- `teei_new_capi_init()` used in `init_teei_framework`
- reserved_mem used for pool of shms
  - TEEC_AllocateSharedMemory and TEEC_RegisterSharedMemory uset his pool

# backward driver

- TEEI_CREAT_BDRV
  - shared param_buf

- boot_stage_1: SMC N_INIT_T_BOOT_STAGE1
  - with boot_vfs_addr shmem
- LOAD_IMG_IRQ --> TEEI_LOAD_IMG_TYPE (BDRV)
  - vfs read boot_vfs_addr --> to userspace daemon
  - after that: userspace daemon --> vfs write --> SMC N_ACK_T_LOAD_IMG
  - Question: WHICH IMAGE goes into here???

# Needed components

- tz_driver
  - exclude:
    - sysfs.c
    - teei_client_transfer_data.c
    - tz_log.c

- tee/soter
- 