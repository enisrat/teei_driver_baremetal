#!/usr/bin/python3

import sys
import re


print("#include \"linux/types.h\"")
      
regex = re.compile(r'U\s*([a-zA-Z_0-9]*)')

# read stdin completely and match regexp
content = sys.stdin.read()
for m in regex.finditer(content):
    match m[1]:
        case "memcpy" | "memset" | "strncmp":
            continue
        case "device_create" :
            print(f"u64 {sym}() {{return 1;}};")
        case sym:
            print(f"u64 {sym}() {{return 0;}};")
