import matplotlib.pyplot as plt
import os
import subprocess
import numpy as np
import math
from prettytable import PrettyTable

matrix_size = 1000 
thread_counts = [1, 2, 4, 8, 16, 32, 64]
array_sizes = [200, 500, 1000]
program = "./main"
results = []
tolerance = 1e-6   

# Function to generate and save deterministic matrix and vector data
def generate_and_save_data(size):
    np.random.seed(42)
    B = np.random.rand(size, size) * 10
    x = np.random.rand(size) * 10
    y = np.random.rand(size) * 10

    np.savetxt("matrix_B.txt", B, fmt="%.6f")
    np.savetxt("vector_x.txt", x, fmt="%.6f")
    np.savetxt("vector_y.txt", y, fmt="%.6f")
    print(f"Generated and saved data for matrix size {size}.")

def run_program(threads, size, use_files=False):
    command = [program, "--threads", str(threads), "--size", str(size)]
    if use_files:
        command.append("--use-files")
    result = subprocess.run(command, capture_output=True, text=True)
    execution_time = None
    computed_result = None

    # Parse the output
    for line in result.stdout.split("\n"):
        if "Execution Time" in line:
            execution_time = float(line.split(":")[1].strip())
        if "Result (d)" in line:
            computed_result = float(line.split(":")[1].strip())

    return execution_time, computed_result

# 1- prepare verification step 
generate_and_save_data(matrix_size)

print("Running sequential version for checking (1 thread, with array size of 1000, With using generated array data)...")
seq_time, seq_result = run_program(1, use_files=True, size=1000)
if seq_time is None or seq_result is None:
    print("Failed to retrieve results for the sequential run.")
    exit()

print(f"Sequential Result: {seq_result}")
print(f"Sequential Execution Time: {seq_time} seconds")

print(f"Running with 8 threads to compare the results with the sequential")
par_time, par_result = run_program(8, use_files=True, size=1000)
if par_time is None or par_result is None:
    print(f"Failed to retrieve results for 8 threads.")
elif math.isclose(seq_result, par_result, rel_tol=tolerance):
    print(f"Validation passed for 8 threads. Result: {par_result}")
else:
    print(f"Validation failed for 8 threads! Sequential: {seq_result}, Parallel: {par_result}")


execution_times = {size: {} for size in array_sizes}  # Store execution times by size and thread count

# 2- running all threads
for size in array_sizes:
    for threads in thread_counts:
        print(f"Running with {size} size {threads} threads...")
        execution_time, computed_result = run_program(threads, use_files=False, size=size)

        print(execution_time)
        if execution_time is not None:
                execution_times[size][threads] = execution_time
        else:
            print(f"Failed to extract execution time for {threads} threads.")


# Calculate speedups
speedups = {size: {} for size in array_sizes}
for size in array_sizes:
    sequential_time = execution_times[size][1]  # Time for 1 thread as the baseline
    for threads in thread_counts:
        speedups[size][threads] = sequential_time / execution_times[size][threads]

# Print speedup tables to console
for size in array_sizes:
    table = PrettyTable()
    table.field_names = ["Threads", "Speedup"]
    for threads in thread_counts:
        table.add_row([threads, round(speedups[size][threads], 2)])
    print(f"\nSpeedup Table for Matrix Size {size}x{size}")
    print(table)


plt.figure(figsize=(10, 6))
for size in array_sizes:
    speedup_values = [speedups[size][threads] for threads in thread_counts]
    plt.plot(thread_counts, speedup_values, marker="o", label=f"Size {size}x{size}")

plt.title("Speedup vs Threads (Acceleration Analysis)")
plt.xlabel("Number of Threads")
plt.ylabel("Speedup (Acceleration)")
plt.xticks(thread_counts)
plt.grid()
plt.legend()
plt.savefig("speedup_vs_threads_analysis.png")
plt.show()


