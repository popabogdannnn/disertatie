#!/usr/bin/python3

import sys
import glob
import os

n0 = 0
n1 = 0
m = 0


for file in glob.glob("tests/exact_tests/*"):
    f = open(file, "r")
    print(file)
    line = f.readline()
    x = line.split()
    n0 = int(x[2])
    n1 = int(x[3])
    m = int(x[4])
    f.close()
    if(n1 <= 4000):
        os.system(f"cp {file} tests/small_exact_tests/")
