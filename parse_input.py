#!/usr/bin/python3

import sys


t = sys.stdin.read()
t = t.split('\n')[:-1]

n0 = 0
n1 = 0
m = 0

for line in t:
    if(line.startswith('c')):
        continue
    if(line.startswith('p')):
        x = line.split()
        n0 = int(x[2])
        n1 = int(x[3])
        m = int(x[4])
        print(n0, n1, m)
    else:
        x = line.split()
        print(int(x[0]) - 1, int(x[1]) - n0 - 1)