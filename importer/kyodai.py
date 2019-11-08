import os
import sys

import ti83f

# This entire file is probably entirely "un-pythonic," whatever that means
# Screw that, I want something that works; this bit doesn't need to be readable.

version = 1

varname = sys.argv[1]

data = bytearray()
num_layouts = 0

for filename in sys.argv[2:]:
    with open(filename, "r") as f:
        layout_name = os.path.basename(filename).split('.')[0][0:15]
        line = f.readlines()[2]
        pos = 0
        slots = list()
        for lev in range(0, 5):
            for row in range(0, 20):
                for col in range(0, 34):
                    if line[pos] == '1':
                        slots.append((col, row, lev))
                    pos += 1
        if len(slots) == 144:
            data += bytearray([1, 41, 0, 0, 15]) + bytearray(layout_name, 'utf-8') + bytearray([0]) + bytearray(
                15 - len(layout_name))
            for i in range(0, 144):
                data += bytearray([slots[i][0], slots[i][1], slots[i][2]])
            num_layouts += 1
        else:
            print("File " + layout_name + " had " + str(len(slots)) + " tiles rather than 144")

print("Converted " + str(num_layouts) + " layouts.")

data = bytearray("MJ", 'utf8') + bytearray([0, 1]) + num_layouts.to_bytes(3, byteorder='little') + data

variable = ti83f.Variable(name=bytes(varname, 'utf-8'), data=bytes(data), archived=True)

appvar = ti83f.AppVar()
appvar.add(variable)

raw_data = bytes(appvar)

with open(varname + '.8xv', 'wb') as f:
    f.write(raw_data)
