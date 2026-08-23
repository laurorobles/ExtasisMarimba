#!/usr/bin/env python3
"""
ExtasisMarimba - License Key Generator & Verifier
Standard CLI for Extasis Records (matching ExtasisRhythm model).

Format: EXTM-XXXX-XXXX-XXXX-XXXX
"""

import sys
import os
import random
import argparse
import hashlib

SALT_1 = 0xB894E28F4C6D39D7
SALT_2 = 0x7F5D91C32E98E46B

def generate_key_from_val1(val1: int) -> str:
    val1 = val1 & 0xFFFF
    seed = (val1 << 32) | val1
    
    # Block 2
    expected2 = (((seed ^ SALT_1) * 0x45D9F3B) >> 16) & 0xFFFF
    
    # Block 3
    rotl = ((seed << 13) | (seed >> 19)) & 0xFFFFFFFFFFFFFFFF
    expected3 = (((rotl ^ SALT_2) * 0x27D4EB2D) >> 16) & 0xFFFF
    
    # Block 4
    expected4 = ((val1 ^ expected2 ^ expected3 ^ 0xCAFE) * 0x119DE1) & 0xFFFF
    
    return f"EXTM-{val1:04X}-{expected2:04X}-{expected3:04X}-{expected4:04X}"

def generate_random_key() -> str:
    while True:
        val1 = random.randint(0x1000, 0xEFFF)
        if val1 != 0:
            return generate_key_from_val1(val1)

def generate_gift_key(name: str) -> str:
    h = hashlib.sha256(name.strip().lower().encode('utf-8')).hexdigest()
    val1 = int(h[:4], 16)
    if val1 == 0:
        val1 = 0x1234
    return generate_key_from_val1(val1)

def verify_key(key: str) -> bool:
    clean = key.strip().upper().replace("-", "").replace(" ", "")
    if clean.startswith("EXTM"):
        clean = clean[4:]
    
    if len(clean) != 16:
        return False
        
    try:
        val1 = int(clean[0:4], 16)
        val2 = int(clean[4:8], 16)
        val3 = int(clean[8:12], 16)
        val4 = int(clean[12:16], 16)
    except ValueError:
        return False
        
    if val1 == 0 and val2 == 0 and val3 == 0:
        return False
        
    seed = (val1 << 32) | val1
    expected2 = (((seed ^ SALT_1) * 0x45D9F3B) >> 16) & 0xFFFF
    rotl = ((seed << 13) | (seed >> 19)) & 0xFFFFFFFFFFFFFFFF
    expected3 = (((rotl ^ SALT_2) * 0x27D4EB2D) >> 16) & 0xFFFF
    expected4 = ((val1 ^ expected2 ^ expected3 ^ 0xCAFE) * 0x119DE1) & 0xFFFF
    
    return (val2 == expected2 and val3 == expected3 and val4 == expected4)

def main():
    parser = argparse.ArgumentParser(description="ExtasisMarimba License Generator")
    parser.add_argument("--single", action="store_true", help="Generate a single random serial key")
    parser.add_argument("--batch", type=int, metavar="N", help="Generate N serial keys")
    parser.add_argument("--gift", type=str, metavar="NAME", help="Generate a personalized gift key for NAME")
    parser.add_argument("--verify", type=str, metavar="KEY", help="Verify if KEY is valid")

    args = parser.parse_args()

    if args.verify:
        is_valid = verify_key(args.verify)
        if is_valid:
            print(f"✅ Key {args.verify} is VALID!")
            sys.exit(0)
        else:
            print(f"❌ Key {args.verify} is INVALID!")
            sys.exit(1)

    if args.gift:
        key = generate_gift_key(args.gift)
        print(f"🎁 Personalized License for [{args.gift}]: {key}")
        return

    if args.batch:
        print(f"--- Generating {args.batch} ExtasisMarimba Licenses ---")
        for i in range(args.batch):
            print(generate_random_key())
        return

    # Default to single
    key = generate_random_key()
    print(f"ExtasisMarimba License: {key}")

if __name__ == "__main__":
    main()
