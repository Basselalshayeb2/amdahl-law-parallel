import matplotlib.pyplot as plt
import os
import subprocess
import numpy as np
import math

matrix_size = 1000 
thread_counts = [1, 2, 4, 8, 16]
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

def run_program(threads, use_files=False):
    os.environ["OMP_NUM_THREADS"] = str(threads)
    command = [program]
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

print("Running sequential version (1 thread, with files)...")
seq_time, seq_result = run_program(1, use_files=True)
if seq_time is None or seq_result is None:
    print("Failed to retrieve results for the sequential run.")
    exit()

print(f"Sequential Result (from files): {seq_result}")
print(f"Sequential Execution Time (from files): {seq_time} seconds")

print(f"Running with 8 threads to compare the results with the sequential")
par_time, par_result = run_program(8, use_files=True)
if par_time is None or par_result is None:
    print(f"Failed to retrieve results for 8 threads.")
elif math.isclose(seq_result, par_result, rel_tol=tolerance):
    print(f"Validation passed for 8 threads. Result: {par_result}")
else:
    print(f"Validation failed for 8 threads! Sequential: {seq_result}, Parallel: {par_result}")


# 2- running all threads
for threads in thread_counts:
    print(f"Running with {threads} threads...")
    os.environ["OMP_NUM_THREADS"] = str(threads)
    result = subprocess.run([program], capture_output=True, text=True)

    # Extract execution time from the output
    execution_time = None
    for line in result.stdout.split("\n"):
        if "Execution Time" in line:
            execution_time = float(line.split(":")[1].strip())
            break

    if execution_time is not None:
        results.append((threads, execution_time))
    else:
        print(f"Failed to extract execution time for {threads} threads.")

if not results:
    print("No results captured. Ensure the C++ program is working correctly.")
    exit()

threads, times = zip(*results)
sequential_time = times[0]
speedups = [sequential_time / time for time in times]

# Execution Time vs Threads
plt.figure(figsize=(10, 5))
plt.plot(threads, times, marker="o", label="Execution Time")
plt.title("Execution Time vs Number of Threads")
plt.xlabel("Number of Threads")
plt.ylabel("Execution Time (seconds)")
plt.grid(True)
plt.legend()
plt.savefig("execution_time_vs_threads.png")
plt.show()

# Speedup vs Threads
plt.figure(figsize=(10, 5))
plt.plot(threads, speedups, marker="o", label="Speedup")
plt.title("Speedup vs Number of Threads (Amdahl's Law)")
plt.xlabel("Number of Threads")
plt.ylabel("Speedup")
plt.grid(True)
plt.legend()
plt.savefig("speedup_vs_threads.png")
plt.show()

print("\nResults:")
print(f"{'Threads':<10}{'Execution Time':<20}{'Speedup':<10}")
for t, time, speedup in zip(threads, times, speedups):
    print(f"{t:<10}{time:<20}{speedup:<10}")
