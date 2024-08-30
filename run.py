#!/usr/bin/python3

import sys, os, glob, time
import subprocess

solutions = sys.argv[2:]
tests = sys.argv[1]

for solution in solutions:
    os.system(f"./compile {solution}")

test_list = glob.glob(f"{tests}")
for test in sorted(test_list):
    print(f"TEST {test}:")
    for solution in solutions:
        print(f"Solution: {solution}")
        start_time = time.time()
        os.system(f"./parse_input.py < {test} | ./obj/{solution} > aux.out")
        end_time = time.time()
        print(f"Execution: {end_time - start_time} seconds")
        pace2024verifier_command = subprocess.run(["pace2024verifier", test, "aux.out"], stdout = subprocess.PIPE)
        crossings = int(pace2024verifier_command.stdout.split()[-6])
        print(f"Crossings = {crossings}")
    print("<------------------->")
    print()
    print()