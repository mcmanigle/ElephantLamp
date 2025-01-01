#!/usr/bin/env python
#
# This Pythons script uses PIL/Pillow to read in an image file
# and convert it to a header file.
#

import math
import argparse
import struct
from PIL import Image

# command line parsing
parser = argparse.ArgumentParser(description='Convert image to header file.')
parser.add_argument('input_image', help='the input image file')
parser.add_argument('-o', '--out_name', help='output name')
args = parser.parse_args()

IN_FILE = args.input_image
NAME = args.out_name if args.out_name else IN_FILE.split('.')[0]

# open image file
print("Reading input file", IN_FILE)
img = Image.open(IN_FILE).convert('RGB')

# create byte chunks
print("Converting to bytes.")

def px_byte(pixel):
    r = (pixel[0] >> 3) & 0x1F
    g = (pixel[1] >> 2) & 0x3F
    b = (pixel[2] >> 3) & 0x1F
    return struct.pack('>H', (r << 11) + (g << 5) + b)

img_bytes = map(px_byte, img.getdata())

# write header file
header_file = NAME+'.h'
print("Writing header file", header_file)
with open(header_file, 'w') as fp:
    fp.write("#ifndef _{}_h_\n".format(NAME))
    fp.write("#define _{}_h_\n\n".format(NAME))
    fp.write("#include <pgmspace.h>\n\n")
    fp.write("#define {}_width {}\n".format(NAME, img.width))
    fp.write("#define {}_height {}\n\n".format(NAME, img.height))
    fp.write("static const unsigned short PROGMEM {}_data[] = ".format(NAME))
    fp.write("{\n")
    row_count = 0
    for b in img_bytes:
        fp.write("0x{}, ".format(b.hex()))
        row_count += 1
        if row_count >= 12:
            fp.write("\n")
            row_count = 0
    fp.write("};\n")
    fp.write("\n#endif // _{}_h_\n".format(NAME))

print("DONE.")