#!/usr/bin/env python
#
# This Pythons script uses PIL/Pillow to read in an image file
# and convert it to a header file.
#

import math
import argparse
import struct
from PIL import Image

IN_DIRECTORY = '/Users/mcmanigle/Code/weather-icons/production/fill/png/512/'
OUTPUT_SIZE = 80

icons_to_grab = ['clear-night',
                 'overcast',
                 'code-yellow',
                 'fog-day',
                 'fog-night',
                 'overcast-hail',
                 'thunderstorms-overcast',
                 'thunderstorms-overcast-rain',
                 'partly-cloudy-day',
                 'partly-cloudy-night',
                 'overcast-rain',
                 'extreme-rain',
                 'overcast-snow',
                 'overcast-sleet',
                 'clear-day',
                 'wind',
                ]

black_background = Image.new('RGBA', (OUTPUT_SIZE, OUTPUT_SIZE))

def px_byte(pixel):
    r = (pixel[0] >> 3) & 0x1F
    g = (pixel[1] >> 2) & 0x3F
    b = (pixel[2] >> 3) & 0x1F
    return struct.pack('>H', (r << 11) + (g << 5) + b)

header_file = 'weather_icons.h'
print("Writing header file", header_file)
with open(header_file, 'w') as fp:

    fp.write("#ifndef _weather_icons_h_\n")
    fp.write("#define _weather_icons_h_\n\n")
    fp.write("#include <pgmspace.h>\n")
    fp.write("#include <TFT_eSPI.h>\n\n")
    fp.write("#define weather_icon_width {}\n".format(OUTPUT_SIZE))
    fp.write("#define weather_icon_height {}\n\n".format(OUTPUT_SIZE))

    fp.write('void draw_weather_icon(TFT_eSprite *sprite, int x, int y, const char *weather_string, bool is_night);\n\n')
    fp.write('void draw_weather_icon_day(TFT_eSprite *sprite, int x, int y, const char *weather_string);\n')
    fp.write('void draw_weather_icon_night(TFT_eSprite *sprite, int x, int y, const char *weather_string);\n\n')

    for icon in icons_to_grab:
        print('Processing '+icon)
        
        img = Image.open(IN_DIRECTORY + icon + '.png').convert('RGBA')
        if img.height != OUTPUT_SIZE:
            print(' - Resizing')
            img = img.resize((OUTPUT_SIZE,OUTPUT_SIZE))

        img = Image.alpha_composite(black_background, img)
        
        img_bytes = map(px_byte, img.getdata())

        fp.write("static const unsigned short PROGMEM {}_data[] = ".format(icon.replace('-','_')))
        fp.write("{\n")
        row_count = 0
        for b in img_bytes:
            fp.write("0x{}, ".format(b.hex()))
            row_count += 1
            if row_count >= 12:
                fp.write("\n")
                row_count = 0
        fp.write("};\n\n")

    fp.write("\n#endif // _weather_icons_h_\n")

print("DONE.")