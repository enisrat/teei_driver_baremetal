#!/usr/bin/python3

import sys
import re


print("#include \"linux/types.h\"")
      
regex = re.compile(r'U\s*([a-zA-Z_0-9]*)')

# read stdin completely and match regexp
content = sys.stdin.read()
for m in regex.finditer(content):
    match m[1]:
        case "memcpy" | "memset" | "strncmp" | "strlen" | "printk" | "__get_free_pages" | "vmalloc" | "__kmalloc" | "virt_to_phys" | "__arm_smccc_smc"| "notify_vfs_handle" | "ktime_get_with_offset":
            continue
        case "get_imsg_log_level":
            print(f"u64 get_imsg_log_level() {{return 8;}};")
        case s if s in ["platform_device_alloc", "device_create"]:
            print(f"u64 {s}() {{return 1;}};")
        case s if s in ["ioremap_cache"]:
            print(f"u64 {s}(u64 a) {{return a;}};")
        case s:
            print(f"u64 {s}() {{return 0;}};")
