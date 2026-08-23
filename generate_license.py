#!/usr/bin/env python3
"""
Extasis Marimba v1.0 — Official Cryptographic License Key Generator & Manager
Use this tool to generate individual serials, gift keys, or batch export for Plugin Boutique / Gumroad / Bandcamp.
"""

import sys
import argparse
import random
import hashlib

SALT_1 = 0x8C3F5E7B1A9D4E20
SALT_2 = 0x5D9B2A1E7F8C4362
MASK_64 = 0xFFFFFFFFFFFFFFFF
MASK_16 = 0xFFFF

def generate_serial_for_seed(val1: int) -> str:
    val1 = val1 & MASK_16
    if val1 == 0:
        val1 = 1
    seed = ((val1 << 32) | val1) & MASK_64
    
    expected2 = (((seed ^ SALT_1) * 0x45D9F3B) & MASK_64) >> 16
    expected2 &= MASK_16
    
    rot = (((seed << 13) & MASK_64) | (seed >> 19)) & MASK_64
    expected3 = (((rot ^ SALT_2) * 0x27D4EB2D) & MASK_64) >> 16
    expected3 &= MASK_16
    
    expected4 = ((val1 ^ expected2 ^ expected3 ^ 0xBEEF) * 0x119DE1) & MASK_16
    
    return f"EXTM-{val1:04X}-{expected2:04X}-{expected3:04X}-{expected4:04X}".upper()

def validate_serial(serial: str) -> bool:
    s = serial.strip().upper().replace("-", "").replace(" ", "").replace("\t", "").replace("\r", "").replace("\n", "")
    if s.startswith("EXTM"):
        s = s[4:]
    if len(s) != 16:
        return False
    try:
        val1 = int(s[0:4], 16)
        val2 = int(s[4:8], 16)
        val3 = int(s[8:12], 16)
        val4 = int(s[12:16], 16)
    except ValueError:
        return False

    if val1 == 0 and val2 == 0 and val3 == 0:
        return False

    seed = ((val1 << 32) | val1) & MASK_64
    expected2 = ((((seed ^ SALT_1) * 0x45D9F3B) & MASK_64) >> 16) & MASK_16
    rot = (((seed << 13) & MASK_64) | (seed >> 19)) & MASK_64
    expected3 = ((((rot ^ SALT_2) * 0x27D4EB2D) & MASK_64) >> 16) & MASK_16
    expected4 = ((val1 ^ expected2 ^ expected3 ^ 0xBEEF) * 0x119DE1) & MASK_16

    return (val2 == expected2 and val3 == expected3 and val4 == expected4)

def generate_gift_serial(name_or_email: str) -> str:
    h = hashlib.sha256(name_or_email.strip().lower().encode('utf-8')).hexdigest()
    seed_val = int(h[:4], 16)
    return generate_serial_for_seed(seed_val)

def main():
    parser = argparse.ArgumentParser(description="Extasis Marimba Serial Key Generator & Manager")
    parser.add_argument("--single", action="store_true", help="Generate a single random serial key")
    parser.add_argument("--gift", type=str, help="Generate a personalized serial key for a user name or email")
    parser.add_argument("--batch", type=int, help="Generate N unique serial keys for Plugin Boutique / Gumroad batch upload")
    parser.add_argument("--output", type=str, default="serials_marimba.txt", help="Output file for batch generation (default: serials_marimba.txt)")
    parser.add_argument("--verify", type=str, help="Verify if a given serial key is valid")

    args = parser.parse_args()

    if args.verify:
        is_val = validate_serial(args.verify)
        if is_val:
            print(f"✅ VALID SERIAL: {args.verify.strip().upper()}")
        else:
            print(f"❌ INVALID SERIAL: {args.verify.strip()}")
        sys.exit(0 if is_val else 1)

    if args.gift:
        key = generate_gift_serial(args.gift)
        print("=" * 60)
        print(f"🎁 EXTASIS MARIMBA — GIFT / CUSTOMER LICENSE KEY")
        print(f"   Recipient:  {args.gift}")
        print(f"   Serial Key: {key}")
        print("=" * 60)
        sys.exit(0)

    if args.batch:
        count = args.batch
        all_seeds = list(range(1, 65535))
        random.shuffle(all_seeds)
        selected = all_seeds[:count]
        serials = [generate_serial_for_seed(s) for s in selected]

        with open(args.output, "w") as f:
            for s in serials:
                f.write(s + "\n")

        print("=" * 60)
        print(f"📦 BATCH EXPORT COMPLETE FOR PLUGIN BOUTIQUE / GUMROAD")
        print(f"   Generated: {len(serials)} unique serial keys")
        print(f"   Saved to:  {args.output}")
        print(f"   First 3 samples:")
        for s in serials[:3]:
            print(f"     -> {s}")
        print("=" * 60)
        sys.exit(0)

    # Default to single key
    seed = random.randint(1, 65534)
    key = generate_serial_for_seed(seed)
    print("=" * 60)
    print(f"🔑 EXTASIS MARIMBA — SERIAL KEY")
    print(f"   {key}")
    print("=" * 60)

if __name__ == "__main__":
    main()
