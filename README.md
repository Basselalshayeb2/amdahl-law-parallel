# Matrix Computation with OpenMP

### Description
This program computes the value of d, defined as:
d = <B4 x, y>/<x, y> - <B3 x, y>/<x, y>

Where:
- B is a square matrix of size `1000` for extrem testing.
- X and Y are vectors.
- <.,.> represents the scalar product

The program supports both sequential **Sequential** and **Parallel** execution using OpenMP.
I added the ability to take data from a file to make a test and verify answers between the two solutins.

---

### Features
- Matrix and vector input can be read from files or generated randomly.
- Parallelized computations for matrix multiplications and scalar products using OpenMP.
- Validation against sequential execution for correctness.

---

### Requirements
- **C++ Compiler with OpenMP Support** 
- **Python 3.10**

---

### Usage
**1. Compilation** \
Compiling the program main.cpp with OpenMP support:
```bash
g++ -o main -fopenmp  main.cpp
```
**2. Execution** \
For full testing I used Python with a couple of predefined threads counts
```
[1, 2, 4, 8, 16]
```

You run the python to see the full diagrams and full tests written there

```bash
python3 analyzer.py
```
Or you can run it manually and specify the number of threads you want 

```bash
export OMP_NUM_THREADS=X
./main
```

**3. Debugging** \
To run sequentialy and check the speed for:
```bash
export OMP_NUM_THREADS=1 
```

---


### Output
1. Using the C++ code the program prints:
- The computed result of d
- The time for calculation and debugging data

2. Using the python code
- All results for each amount of threads
- Diagram to show Speed Up X Threads relatio


---

### Result after running with array size of 1000
1. Testing environment