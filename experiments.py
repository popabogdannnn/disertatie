# %%
import sys, os, glob, time
import matplotlib.pyplot as plt
import subprocess

PROCS = [1, 8, 16, 32]

# %%
params_bitmask_low_memo = [
    [3000, 2, 2000, 1],
    [3000, 3, 2000, 1],
    [3000, 4, 2000, 1],
    [3000, 5, 2000, 1],
    [3000, 6, 2000, 1],
    [3000, 7, 2000, 1],
    [3000, 8, 2000, 1],
    [3000, 9, 2000, 1],
    [3000, 10, 2000, 1],
    [3000, 11, 2000, 1],
    [3000, 12, 2000, 1],
    [3000, 13, 2000, 1],
    [3000, 14, 2000, 1],
    [3000, 15, 2000, 1],
    [3000, 16, 2000, 1],
    [3000, 17, 2000, 1],
    [3000, 18, 2000, 1],
    [3000, 19, 2000, 1],
    [3000, 20, 3000, 1],
    [3000, 21, 3000, 1],
    [3000, 22, 3000, 1],
    [3000, 23, 3000, 1],
    [3000, 24, 3000, 1],
    [3000, 25, 3000, 1],
    [3000, 26, 3000, 1],
    [3000, 27, 4000, 1],
]
params_bitmask = params_bitmask_low_memo + [
    [3000, 28, 4000, 1],
    [3000, 29, 4000, 1],
    [3000, 30, 4000, 1],
] 


# %%
def generate_tests(params, test_name):
    os.system("g++ -std=c++20 -O2 gen.cpp -o gen")
    index = 0
    test_cases = []
    for param in params:
        tag = str(index)
        if(len(tag) == 1):
            tag = "0" + tag
        os.system(f"./gen {param[0]} {param[1]} {param[2]} {param[3]} > {test_name.format(tag)}")
        test_cases.append(test_name.format(tag))
        index += 1
    return test_cases
# generate_tests(params_bitmask, "tests/bitmask_tests/{}_random_bitmask.gr")

# %%
def run_solution_on_test(solution, test_case, PROCS):
    print(f"Running solution {solution} on {test_case}")

    if not os.path.exists(f"{test_case}_stats"): 
        os.makedirs(f"{test_case}_stats") 

    # --- First run: clean timing ---
    f_in = open(test_case, "r")
    f_out = open("aux.out", "w")
    start_time = time.time()
    parse_command = subprocess.Popen(["./parse_input.py"], stdin=f_in, stdout=subprocess.PIPE)
    solution_command = subprocess.Popen(
        [f"./obj/{solution}", str(PROCS)],
        stdin=parse_command.stdout,
        stdout=f_out,
        stderr=subprocess.PIPE
    )
    parse_command.stdout.close()  # close in parent
    solution_command.communicate()
    end_time = time.time()
    execution_time = end_time - start_time
    f_in.close()
    f_out.close()

    # --- Verify correctness ---
    pace2024verifier_command = subprocess.run(
        ["pace2024verifier", test_case, "aux.out"], stdout=subprocess.PIPE
    )
    crossings = int(pace2024verifier_command.stdout.split()[-6])

    # --- Second run: with perf ---
    f_in = open(test_case, "r")
    f_out = open("aux.out", "w")
    f_err = open(f"{test_case}_stats/{PROCS}_{solution}.run", "w")
    parse_command = subprocess.Popen(["./parse_input.py"], stdin=f_in, stdout=subprocess.PIPE)
    solution_command = subprocess.Popen(
        [
            "perf", "stat",
            "-e", "task-clock,context-switches,cpu-migrations,page-faults,cycles,instructions,branches,branch-misses",
            "-e", "cache-references,cache-misses",
            "-e", "L1-dcache-loads,L1-dcache-load-misses",
            "-e", "LLC-loads,LLC-load-misses",
            "-e", "LLC-stores,LLC-store-misses",
            f"./obj/{solution}", str(PROCS)
        ],
        stdin=parse_command.stdout,
        stdout=f_out,
        stderr=f_err
    )
    parse_command.stdout.close()  # close in parent
    solution_command.communicate()
    f_in.close()
    f_out.close()
    f_err.close()

    return execution_time, crossings

def benchmark(solutions, test_cases, PROCS):
    execution_times = dict()
    crossings = dict()

    os.system(f"./compile {solutions[1]}")
    run_data = []
    for test in test_cases:
        run_data.append(run_solution_on_test(solutions[1], test, "0"))
        print(f"{solutions[1]}: {test}")
    execution_times[solutions[1]] = [t for (t, x) in run_data]
    crossings[solutions[1]] = [x for (t, x) in run_data]
    os.system(f"./compile {solutions[0]}")
    for PROC in PROCS:
    
        run_data = []
        for test in test_cases:
            run_data.append(run_solution_on_test(solutions[0], test, PROC))
            print(f"{solutions[0]} {PROC}: {test}")
        execution_times[f"{PROC}_{solutions[0]}"] = [t for (t, x) in run_data]
        crossings[f"{PROC}_{solutions[0]}"] = [x for (t, x) in run_data]

    
        # assert crossings[solutions[0]] == crossings[solutions[1]]
    
    return execution_times, crossings


# %%
def plot(_info, _fast_solution, _slow_solution, PROCS):
    
    for PROC in PROCS:
        colors = 'brg'
        markers = 'oxt'
        idx = 0
        fast_solution = f"{PROC}_{_fast_solution}"
        slow_solution = _slow_solution
        info = {}
        info[fast_solution] = _info[fast_solution]
        info[slow_solution] = _info[slow_solution]
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 6))
        for (sol, times) in info.items():
            n_values = list(range(2, 2 + len(times)))
            ax1.plot(n_values, times, marker=markers[idx], linestyle='-', color=colors[idx], label=sol)
            idx += 1
        
        ax1.set_xlabel('n')
        ax1.set_ylabel('Execution Time (seconds)')
        ax1.set_title('Execution Time vs n')

        speedup = [info[slow_solution][i] / info[fast_solution][i] for i in range(len(n_values))]

        ax2.plot(n_values, speedup, marker='o', linestyle='-', color='b')
        ax2.set_xlabel('n')
        ax2.set_ylabel('Speedup')
        ax2.set_title('Speedup vs n')

        ax1.legend()

        # Ensure imgs directory exists
        os.makedirs("./imgs", exist_ok=True)

        # Save the figure
        filename = f"./imgs/{fast_solution}_{slow_solution}.png"
        plt.savefig(filename, bbox_inches="tight")

        plt.close(fig)  # Close to free memory



# %%
import numpy as np
def plot_histogram(_info, _fast_solution, _slow_solution, test_cases, PROCS):
    
    for PROC in PROCS:
        colors = 'brg'
        markers = 'oxt'
        idx = 0
        fast_solution = f"{PROC}_{_fast_solution}"
        slow_solution = _slow_solution
        tests = [t.split('/')[-1] for t in test_cases]
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 6))
        bar_width = 0.5
        spacing = 0.3
        delta = 0
        index = np.arange(len(tests)) * (1 + spacing)
        info = {}
        info[fast_solution] = _info[fast_solution]
        info[slow_solution] = _info[slow_solution]
        for (sol, times) in info.items():
            ax1.bar(index + delta, times, bar_width, color=colors[idx], label=sol, edgecolor='black')
            delta += bar_width
            idx += 1
        
        ax1.set_xlabel('Test case')
        ax1.set_ylabel('Execution Time (seconds)')
        ax1.set_title('Execution Time vs test case')
        ax1.set_xticks(index + bar_width / 2)
        ax1.set_xticklabels(tests, rotation=45)

        speedup = [info[slow_solution][i] / info[fast_solution][i] for i in range(len(tests))]

        ax2.bar(index + delta, speedup, bar_width, color='b', edgecolor='black')
        ax2.set_xlabel('Test case')
        ax2.set_ylabel('Speedup')
        ax2.set_title('Speedup vs test case')
        ax2.set_xticks(index + bar_width / 2)
        ax2.set_xticklabels(tests, rotation=45)
        

        ax1.legend()

        plt.tight_layout()
        filename = f"./imgs/{fast_solution}_{slow_solution}.png"
        plt.savefig(filename, bbox_inches="tight")

        plt.close(fig)  # Close to free memory



test_cases = glob.glob("tests/bitmask_tests/*.gr")
test_cases.sort()
data = benchmark(["opt_memo_parallel_bitmask_dp", "opt_memo_bitmask_dp"], test_cases, PROCS)

# %%
plot(data[0], "opt_memo_parallel_bitmask_dp", "opt_memo_bitmask_dp", PROCS)

# %%
test_cases = glob.glob("tests/bitmask_tests/*.gr")
test_cases.sort()
test_cases = test_cases[:-3]
data = benchmark(["parallel_bitmask_dp", "bitmask_dp"], test_cases, PROCS)

# %%
plot(data[0], "parallel_bitmask_dp", "bitmask_dp", PROCS)

# %%
# test_cases = glob.glob("tests/bitmask_tests/*.gr")
# test_cases.sort()
# test_cases = test_cases[0:-3]
# data = benchmark(["opt_memo_parallel_bitmask_dp", "parallel_bitmask_dp"], test_cases, PROCS)

# # %%
# plot(data[0], "opt_memo_parallel_bitmask_dp", "parallel_bitmask_dp", PROCS)

# %%
# test_cases = glob.glob("tests/bitmask_tests/*.gr")
# test_cases.sort()
# data = benchmark(["slow_parallel_bitmask_dp", "slow_bitmask_dp"], test_cases, PROCS)

# %%
# plot(data[0], "slow_parallel_bitmask_dp", "slow_bitmask_dp", PROCS)

# %%
# test_cases = glob.glob("tests/fpt_subexpo_r_tests/*.gr")
# test_cases.sort()
# data = benchmark(["parallel_fpt_subexpo", "fpt_subexpo"], test_cases, PROCS)

# %%
# plot_histogram(data[0], "parallel_fpt_subexpo", "fpt_subexpo", test_cases, PROCS)

# %%
# test_cases = glob.glob("tests/fpt_golden_ratio_r_tests/*.gr")
# test_cases.sort()
# data = benchmark(["parallel_fpt_golden_ratio", "fpt_golden_ratio"], test_cases, PROCS)

# %%
# plot_histogram(data[0], "parallel_fpt_golden_ratio", "fpt_golden_ratio", test_cases, PROCS)
