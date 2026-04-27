#!/usr/bin/env python3

import getopt
import os
import sys
from PIL import Image

BITES_PER_LINE = 16

grayscale_bits = 1
invert = 0
flip = 0

opts, args = getopt.getopt(
    sys.argv[1:], "hi:o:", ["inputfile=", "outputfile=", "grayscale=", "invert=", "flip="]
)

inputfile = None
outputfile = None

def safe_int(v, default=0):
    try:
        return int(v)
    except:
        return default

for opt, arg in opts:
    if opt in ("-i", "--inputfile"):
        inputfile = arg
    elif opt in ("-o", "--outputfile"):
        outputfile = arg
    elif opt == "--grayscale":
        grayscale_bits = safe_int(arg, 1)
    elif opt == "--invert":
        invert = safe_int(arg, 0)
    elif opt == "--flip":
        flip = safe_int(arg, 0)

if not inputfile or not outputfile:
    print("missing input/output")
    sys.exit(1)

img = Image.open(inputfile).convert("L")

if flip:
    img = img.transpose(Image.FLIP_TOP_BOTTOM)

pixels = list(img.get_flattened_data())
width, height = img.size

f = open(outputfile, "w")
name = os.path.basename(outputfile).rsplit(".h", 1)[0]

f.write(f"// {width}x{height}\n")
f.write(f"const unsigned char {name}[] PROGMEM = {{\n ")

line_count = 0

# =========================
# 1-BIT MODE (PACKED)
# =========================
if grayscale_bits == 1:

    for y in range(height):

        byte = 0
        bit_index = 0

        for x in range(width):

            p = pixels[y * width + x]

            if invert:
                p = 255 - p

            if p > 127:
                byte |= (0x80 >> bit_index)

            bit_index += 1

            if bit_index == 8:
                f.write(f" 0x{byte:02x},")
                byte = 0
                bit_index = 0

                line_count += 1
                if line_count == BITES_PER_LINE:
                    f.write("\n ")
                    line_count = 0

        if bit_index != 0:
            f.write(f" 0x{byte:02x},")
            line_count += 1
            if line_count == BITES_PER_LINE:
                f.write("\n ")
                line_count = 0

# =========================
# 4-BIT MODE (REAL NIBBLE PACKING)
# =========================
elif grayscale_bits == 4:

    for y in range(height):
        for x in range(width):

            p = pixels[y * width + x]

            if invert:
                p = 255 - p

            # 0–255 → 0–15 → back to 0–255 (clean 16 levels)
            p = (p >> 4) * 17

            f.write(f" 0x{p:02x},")

            line_count += 1
            if line_count == BITES_PER_LINE:
                f.write("\n ")
                line_count = 0

    f.write("\n")

else:
    raise RuntimeError("grayscale must be 1 or 4")

f.write("\n};\n")
f.close()
