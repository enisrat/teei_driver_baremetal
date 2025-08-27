#!/usr/bin/python3

import sys
import re


print("#include \"linux/types.h\"")
      
regex = re.compile(r'U\s*([a-zA-Z_0-9]*)')

# read stdin completely and match regexp
content = sys.stdin.read()
for m in regex.finditer(content):
    match m[1]:
        case "memcpy" | "memset" | "strncmp" | "printk" | "__get_free_pages" | "vmalloc" | "__kmalloc" | "virt_to_phys" | "__arm_smccc_smc":
            continue
        case "device_create" :
            print(f"u64 device_create() {{return 1;}};")
        case "get_imsg_log_level":
            print(f"u64 get_imsg_log_level() {{return 8;}};")
        case "platform_device_alloc":
            print(f"u64 platform_device_alloc() {{return 1;}};")
        case sym:
            print(f"u64 {sym}() {{return 0;}};")
