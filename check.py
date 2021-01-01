#!/usr/bin/env python3

import json
import sys

def ordered(obj):
    if isinstance(obj, dict):
        return sorted((k, ordered(v)) for k, v in obj.items())
    if isinstance(obj, list):
        return sorted(ordered(x) for x in obj)
    else:
        return obj

print("loading stdin")
data = json.load(sys.stdin)
print(f"loading {sys.argv[1]}")
reference = json.load(open(sys.argv[1]))
print("checking...")
data = ordered(data)
reference = ordered(reference)
if data != reference:
    print("Result does not match reference.")
    #print(f"reference={reference}")
    #print(f"data={data}")
    print("dumping data...")
    json.dump(data, open("std.json", "w"), indent=4)
    json.dump(reference, open("ref.json", "w"), indent=4)
    sys.exit(1)
else:
    print("Correct!")
    sys.exit(0)
    
