#!/usr/bin/python3

import sys
import glob
import os
import time

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
        print(f"{file} done")
        if(s == 0):
            os.system(f"cp {file} tests/fpt_subexpo_tests/")


def filter_golden_ratio():
    mins = 10
    for file in glob.glob("tests/exact_tests/*"):
        start_time = time.time()
        os.system(f"./parse_input.py < {file} | timeout {mins}m ./obj/parallel_fpt_golden_ratio > aux.out")
        end_time = time.time()
        
        print(f"{file} done, Execution time: {end_time - start_time}")
        if(end_time - start_time <= mins * 60 - 1):
            os.system(f"cp {file} tests/fpt_golden_ratio_tests/")
            print("Copied!")
        print("<--->")
filter_golden_ratio()