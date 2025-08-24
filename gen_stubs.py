#!/usr/bin/python3

import sys
import re


print("#include \"linux/types.h\"")
      
regex = re.compile(r'U\s*([a-zA-Z_0-9]*)')

# read stdin completely and match regexp
content = sys.stdin.read()
for match in regex.finditer(content):
    print(f"u64 {match[1]}() {{return 0;}};")
