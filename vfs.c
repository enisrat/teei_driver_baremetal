#include <linux/types.h>

#include "thh/ta/c09c9c5daa504b78b0e46eda61556c3a.h"
#include "thh/ta/8b22aba81ef0ccbfd9f5f4b634127e15.h"
#include "thh/ta/d91f322ad5a441d5955110eda3272fc0.h"
#include "thh/ta/0102030405060708090a0b0c0d0e0f10.h"
#include "thh/ta/c1882f2d885e4e13a8c8e2622461b2fa.h"
#include "thh/ta/93feffccd8ca11e796c7c7a21acb4932.h"
#include "thh/ta/rpmb.h"

extern char *daulOS_VFS_share_mem;

// Define types
typedef int64_t i64;

// Enum for command IDs
typedef enum {
    TZ_VFS_OPEN_CMD = 0x11,
    TZ_VFS_READ_CMD = 0x12,
    TZ_VFS_WRITE_CMD = 0x13,
    TZ_VFS_IOCTL_CMD = 0x14,
    TZ_VFS_CLOSE_CMD = 0x15,
    TZ_VFS_TRUNC_CMD = 0x16,
    TZ_VFS_UNLINK_CMD = 0x17,
    TZ_VFS_LSEEK_CMD = 0x18,
    TZ_VFS_RENAME_CMD = 0x19,
    TZ_VFS_MKDIR_CMD = 0x21,
    TZ_VFS_RMDIR_CMD = 0x22,
    TZ_VFS_OPENDIR_CMD = 0x23,
    TZ_VFS_READDIR_CMD = 0x24,
    TZ_VFS_CLOSEDIR_CMD = 0x25,
    TZ_VFS_COPY_CMD = 0x26,
    TZ_VFS_PRONODE_CMD = 0x30,
    TZ_VFS_SETPROP_CMD = 0x31,
    TZ_VFS_GETPROP_CMD = 0x32,
    TZ_RPMB_OPEN_CMD = 0x300,
    TZ_RPMB_GET_CNT_CMD = 0x301,
    TZ_RPMB_PROGRAM_KEY_CMD = 0x302,
    TZ_RPMB_UNKNOWN_CMD = 0x303,
    TZ_RPMB_READ_CMD = 0x304,
    TZ_RPMB_WRITE_CMD = 0x305,
    TZ_RPMB_CLOSE_CMD = 0x306,
    TZ_RPMB_COMMAND_CMD = 0x310
} TZ_VFS_CMD;

// Structure for shared memory (bidirectional parameters and buffer)
typedef struct {
    u32 cmdresp;    // Input: command ID; Output: result (fd, bytes, status, etc.)
    u32 _pad0;
    u32 param1;     // Parameter 1 (e.g., fd, flags, etc.)
    u32 param2;     // Parameter 2 (e.g., mode, count, cmd, etc.)
    u32 param3;     // Parameter 3 (e.g., whence for lseek, count for readdir, etc.)
    u32 _pad[(4096 / 4) - 5] ;
    char buffer[]; // Flexible array for paths, data buffers, dirents, etc. (assume sufficient size, e.g., 8192+ bytes)
} vfs_shm;

static char *current = 0;
static char *next_chunk = 0;
static int remaining = 0;
static int total = 0;

// Simplified function (ignores unused parameters a2, a3, a4, a5 as they are not relevant after removing logging and properties)
int notify_vfs_handle(void) {
    // Default return size (header size for responses without data in buffer)
    u32 result = 0;

    if (daulOS_VFS_share_mem == NULL)
        return -1;

    vfs_shm *shm = daulOS_VFS_share_mem;
    char *buf = shm->buffer;
    TZ_VFS_CMD cmd = (TZ_VFS_CMD)shm->cmdresp;
    shm->cmdresp = 0;

    switch (cmd) {
        case TZ_VFS_OPEN_CMD:
            // Handle file open
            printk("TZ_VFS_OPEN_CMD: %s\n", shm->buffer);

            current = 0;
            remaining = 0;
            total = 0;
            next_chunk = 0;
            if (!strcmp(buf, "/vendor/thh/ta/c09c9c5daa504b78b0e46eda61556c3a.ta"))
            {
                next_chunk = thh_ta_c09c9c5daa504b78b0e46eda61556c3a_ta;
                remaining = thh_ta_c09c9c5daa504b78b0e46eda61556c3a_ta_len;
                shm->cmdresp = 1;
            }
            //load 8b22aba81ef0ccbfd9f5f4b634127e15 accordingly
            if(!strcmp(buf, "/vendor/thh/ta/8b22aba81ef0ccbfd9f5f4b634127e15.ta")){
                next_chunk = thh_ta_8b22aba81ef0ccbfd9f5f4b634127e15_ta;
                remaining = thh_ta_8b22aba81ef0ccbfd9f5f4b634127e15_ta_len;
                shm->cmdresp = 1;
            }
            if(!strcmp(buf, "/vendor/thh/ta/d91f322ad5a441d5955110eda3272fc0.ta")){
                next_chunk = thh_ta_d91f322ad5a441d5955110eda3272fc0_ta;
                remaining = thh_ta_d91f322ad5a441d5955110eda3272fc0_ta_len;
                shm->cmdresp = 1;
            }
            if(!strcmp(buf, "/vendor/thh/ta/0102030405060708090a0b0c0d0e0f10.ta")){
                next_chunk = thh_ta_0102030405060708090a0b0c0d0e0f10_ta;
                remaining = thh_ta_0102030405060708090a0b0c0d0e0f10_ta_len;
                shm->cmdresp = 1;
            }
            if(!strcmp(buf, "/vendor/thh/ta/c1882f2d885e4e13a8c8e2622461b2fa.ta")){
                next_chunk = thh_ta_c1882f2d885e4e13a8c8e2622461b2fa_ta;
                remaining = thh_ta_c1882f2d885e4e13a8c8e2622461b2fa_ta_len;
                shm->cmdresp = 1;
            }
            if(!strcmp(buf, "/vendor/thh/ta/93feffccd8ca11e796c7c7a21acb4932.ta")){
                next_chunk = thh_ta_93feffccd8ca11e796c7c7a21acb4932_ta;
                remaining = thh_ta_93feffccd8ca11e796c7c7a21acb4932_ta_len;
                shm->cmdresp = 1;
            }

            if(!strcmp(buf, "/data/vendor/thh/system/rpmb.txt")){
                next_chunk = thh_ta_rpmb;
                remaining = thh_ta_rpmb_len;
                shm->cmdresp = 1;
            }
            current = next_chunk;
            total = remaining;

            break;

        case TZ_VFS_CLOSE_CMD:
            printk("TZ_VFS_CLOSE_CMD: %d\n", shm->param1);
            break;

        case TZ_VFS_READ_CMD:
            // Handle file read (data read into buffer)
            printk("TZ_VFS_READ_CMD: %d %d %d\n", shm->param1, shm->param2, shm->param3);
            int sz = shm->param2;
            if (sz > remaining)
                sz = remaining;
            memcpy(buf, next_chunk, sz);
            next_chunk += sz;
            remaining -= sz;
            shm->cmdresp = sz;
            break;

        case TZ_VFS_WRITE_CMD:
            // Handle file write (data from buffer)
            printk("TZ_VFS_WRITE_CMD: %d %d\n", shm->param1, shm->param2);
            break;

        case TZ_VFS_IOCTL_CMD:
            // Handle ioctl (argument in/out via buffer)
            printk("TZ_VFS_IOCTL_CMD: %d\n", shm->param1);
            {
                unsigned long ioctl_cmd = shm->param2;
                switch (shm->param2) {
                    case 112:
                        ioctl_cmd = 0xC0045470ULL;
                        break;
                    case 113:
                        ioctl_cmd = 0xC0045471ULL;
                        break;
                    case 117:
                        ioctl_cmd = 0xC0045475ULL;
                        break;
                    default:
                        // Use param2 as-is
                        break;
                }
                result = -1;
                shm->cmdresp = result;
            }
            break;

        case TZ_VFS_TRUNC_CMD:
            printk("TZ_VFS_TRUNC_CMD\n");
            break;

        case TZ_VFS_UNLINK_CMD:
            printk("TZ_VFS_UNLINK_CMD: %s\n", shm->buffer);
            break;

        case TZ_VFS_LSEEK_CMD:
            printk("TZ_VFS_LSEEK_CMD: %d %d %d\n", shm->param1, shm->param2, shm->param3);
            if(shm->param3 == 2) {
                shm->cmdresp = total;
            }
            if(shm->param3 == 0) {
                next_chunk = current + shm->param2;
                remaining = total - (next_chunk - current);
            }
            break;

        case TZ_VFS_RENAME_CMD:
            printk("TZ_VFS_RENAME_CMD: %s %s\n", shm->buffer, shm->buffer + strlen(shm->buffer) + 1);
            break;

        case TZ_VFS_MKDIR_CMD:
            printk("TZ_VFS_MKDIR_CMD: %s\n", shm->buffer);
            break;

        case TZ_VFS_RMDIR_CMD:
            printk("TZ_VFS_RMDIR_CMD: %s\n", shm->buffer);
            break;

        case TZ_VFS_OPENDIR_CMD:
            printk("TZ_VFS_OPENDIR_CMD: %s\n", shm->buffer);
            break;

        case TZ_VFS_READDIR_CMD:
            printk("TZ_VFS_READDIR_CMD: %d\n", shm->param1);
            break;

        case TZ_VFS_CLOSEDIR_CMD:
            printk("TZ_VFS_CLOSEDIR_CMD\n");
            break;

        case TZ_VFS_COPY_CMD:
            printk("TZ_VFS_COPY_CMD: %s %s\n", shm->buffer, shm->buffer + strlen(shm->buffer) + 1);
            break;

        case TZ_VFS_PRONODE_CMD:
            printk("TZ_VFS_PRONODE_CMD\n");
            break;

        case TZ_VFS_SETPROP_CMD:
            printk("TZ_VFS_SETPROP_CMD: %s %s\n", shm->buffer, shm->buffer + strlen(shm->buffer) + 1);
            break;

        case TZ_VFS_GETPROP_CMD:
            printk("TZ_VFS_GETPROP_CMD: %s\n", shm->buffer);
            break;

        case TZ_RPMB_OPEN_CMD:
            printk("TZ_RPMB_OPEN_CMD\n");
            break;

        case TZ_RPMB_GET_CNT_CMD:
            printk("TZ_RPMB_GET_CNT_CMD\n");
            break;

        case TZ_RPMB_PROGRAM_KEY_CMD:
            printk("TZ_RPMB_PROGRAM_KEY_CMD: %s\n", shm->buffer);
            break;

        case TZ_RPMB_UNKNOWN_CMD:
            printk("TZ_RPMB_UNKNOWN_CMD\n");
            break;

        case TZ_RPMB_READ_CMD:
            printk("TZ_RPMB_READ_CMD: %d\n", shm->param1);
            break;

        case TZ_RPMB_WRITE_CMD:
            printk("TZ_RPMB_WRITE_CMD: %d\n", shm->param1);
            break;

        case TZ_RPMB_CLOSE_CMD:
            printk("TZ_RPMB_CLOSE_CMD\n");
            break;

        case TZ_RPMB_COMMAND_CMD:
            printk("TZ_RPMB_COMMAND_CMD\n");
            break;

        default:
            printk("Unsupported command: %d\n", cmd);
            shm->cmdresp = -95;
            break;
    }
    return 0;
}
