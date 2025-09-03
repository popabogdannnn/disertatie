#!/usr/bin/python3

import os
import subprocess
import time

params = [
    # [3000, 300, 2200, 26],
    # [3000, 250, 2505, 28],
    # [3000, 175, 2100, 30],
    # [3000, 2567, 37865, 22],
    [3000, 125, 1300, 30],
    [3000, 3000, 56252, 22],
]

os.system("g++ -std=c++20 -O2 gen.cpp -o gen")
os.system("./compile parallel_fpt_subexpo")
delta = 4
index = 0
while index < len(params):
    param = params[index]
    tag = str(index + delta)
    if(len(tag) == 1):
        tag = "0" + tag
    
    # Generate testcase
    os.system(f"./gen {param[0]} {param[1]} {param[2]} {param[3]} > tests/fpt_subexpo_r_tests/r_{tag}.gr")
    
    # Run solution and measure execution time
    start_time = time.time()
    try:
        with open(f"tests/fpt_subexpo_r_tests/r_{tag}.gr", "r") as f_in:
            with open(os.devnull, "w") as f_out:  # Discard output
                parse_command = subprocess.Popen(["./parse_input.py"], stdin=f_in, stdout=subprocess.PIPE)
                solution_command = subprocess.Popen([f"./obj/parallel_fpt_subexpo"], 
                                                  stdin=parse_command.stdout, 
                                                  stdout=f_out)
                parse_command.stdout.close()
                
                # Wait for completion with timeout
                try:
                    solution_command.communicate(timeout=300)  # 5 minute timeout
                    end_time = time.time()
                    execution_time = end_time - start_time
                    
                    # Check if execution time is between 1 and 5 minutes
                    if 10 <= execution_time <= 300:  # 1 minute to 5 minutes
                        print(f"Test {tag}: Execution time {execution_time:.2f}s - OK")
                        index += 1  # Move to next testcase
                    else:
                        print(f"Test {tag}: Execution time {execution_time:.2f}s - Regenerating...")
                        # Will regenerate with same parameters in next iteration
                        
                except subprocess.TimeoutExpired:
                    print(f"Test {tag}: Timeout (>5 minutes) - Regenerating...")
                    solution_command.kill()
                    parse_command.kill()
                    # Will regenerate with same parameters in next iteration
                    
    except Exception as e:
        print(f"Test {tag}: Error running solution: {e} - Regenerating...")
        # Will regenerate with same parameters in next iteration