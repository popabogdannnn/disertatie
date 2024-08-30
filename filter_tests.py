#!/usr/bin/python3

import sys
import glob
import os

def filter_small_exact():
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

def filter_fpt():
    for file in glob.glob("tests/exact_tests/*"):
        s = os.system(f"./parse_input.py < {file} | ./obj/fpt_subexpo > aux.out")
        if(s == 0):
            os.system(f"cp {file} tests/fpt_subexpo_tests/")

filter_fpt()