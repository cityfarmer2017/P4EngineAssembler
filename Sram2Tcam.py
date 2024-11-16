#!/bin/python3

import sys
import os
import re

c4_c2_map = {
    "1000": "00",
    "0100": "01",
    "0010": "10",
    "0001": "11",
    "1100": "0x",
    "1010": "x0",
    "0011": "1x",
    "0101": "x1",
    "1111": "xx",
}

TCAM_ENTRY_CNT = 32
MAX_ENTRY_NO = TCAM_ENTRY_CNT - 1
TOTAL_STATE_CNT = 32
BIT_CNT_PER_STATE_PER_RAM = 4
TOTAL_BIT_CNT_PER_RAM = TOTAL_STATE_CNT * BIT_CNT_PER_STATE_PER_RAM
TOTAL_RAM_CHIP_CNT = 20
TOTAL_RAM_LINE_CNT = TOTAL_BIT_CNT_PER_RAM * TOTAL_RAM_CHIP_CNT

if len(sys.argv) != 2:
    print("this script shall take exactly one argument of input path to source file!")
    exit(1)

in_fpath = sys.argv[1]
if not os.path.exists(in_fpath):
    print("the input path to source file does not exist!")
    exit(1)

# in_fname = os.path.basename(in_fpath)
data = []
with open(in_fpath, 'r') as file:
    lines = file.readlines()
    for line in lines:
        if re.match(r'^\s*\/\/.*$', line.strip()):
            continue
        if not re.match(r'^[01]{32}$', line.strip()):
            exit(1)
        data.append(line.strip())

if len(data) != TOTAL_RAM_LINE_CNT:
    print("input data file wrong!")
    exit(1)

for column in reversed(range(TCAM_ENTRY_CNT)):
    trans_data = ''.join([row[column] for row in data])
    for state_id in range(0, TOTAL_BIT_CNT_PER_RAM, BIT_CNT_PER_STATE_PER_RAM):
        ot_path = os.path.dirname(in_fpath) if len(os.path.dirname(in_fpath)) else os.path.curdir
        ot_path += "/" + str(state_id // BIT_CNT_PER_STATE_PER_RAM).rjust(3, '0') + ".tcam"
        file = open(ot_path, 'w') if column == MAX_ENTRY_NO else open(ot_path, 'a')
        entry = []
        for ram_id in range(TOTAL_RAM_CHIP_CNT):
            start = ram_id * TOTAL_BIT_CNT_PER_RAM + state_id
            substr = trans_data[start : start + BIT_CNT_PER_STATE_PER_RAM]
            entry.append(c4_c2_map[substr])
        entry.reverse()
        file.write(''.join(entry) + "\n")
