# Assignment-5: SimpleMultithreader

## Group Information
- **Group ID:** [Your Group ID]
- **Member 1:** [Name, Roll Number] - 50% contribution
- **Member 2:** [Name, Roll Number] - 50% contribution
- **Private GitHub Repository Link:** [Link to your private GitHub repo]

---

## 1. Overview
The `SimpleMultithreader` library (`simple-multithreader.h`) provides an easy-to-use abstraction over POSIX threads (`pthreads`) for parallelizing loops in C++ using C++11 lambda expressions. It implements two main functions:
1. `parallel_for` (1D): Parallelizes one-dimensional loops `[low, high)`.
2. `parallel_for` (2D): Parallelizes two-dimensional nested loops `[low1, high1)` x `[low2, high2)`.

---

## 2. Design & Implementation Details

### Thread Management & Exact Thread Count
- As required by the specification, the total number of threads during parallel execution strictly matches `numThreads` (including the main thread).
- If `numThreads == 1`, the loop executes sequentially directly on the main thread (0 pthreads created).
- If `numThreads > 1`, exactly `numThreads - 1` worker pthreads are created via `pthread_create`.
- The main thread directly executes the final chunk (`thread index = numThreads - 1`).
- The main thread then calls `pthread_join` on all `numThreads - 1` worker threads before returning.
- All threads terminate as soon as the `parallel_for` function finishes execution (no persistent thread pools).

### Work Distribution (Chunking)
- To balance iterations evenly across `numThreads`:
  - `chunkSize = total / numThreads`
  - `remainder = total % numThreads`
- The first `remainder` threads receive `chunkSize + 1` iterations, and the remaining threads receive `chunkSize` iterations.
- This guarantees:
  - Every iteration in `[low, high)` is executed exactly once.
  - No iteration is skipped or duplicated.
  - No out-of-bounds array access.
- For 2D loops, the outer loop iterations `[low1, high1)` are divided among threads using the same balanced chunking strategy, and each thread processes its assigned rows across all columns `[low2, high2)`.

### Execution Timing
- Wall-clock execution time is measured for each call of `parallel_for` using `std::chrono::high_resolution_clock`.
- The elapsed time is printed in milliseconds (`ms`) at the end of each `parallel_for` invocation.

### Error Handling & Edge Cases
- Validates that `numThreads > 0`.
- Handles empty or invalid ranges (`low >= high` or `low1 >= high1 || low2 >= high2`) safely without errors.
- Handles cases where `numThreads > total_iterations` gracefully.
- Checks return codes of `pthread_create` and `pthread_join`. If a thread creation fails, remaining chunks are safely executed by the main thread.
- Memory allocated for thread arguments and thread identifiers is cleaned up with `delete[]`.

---

## 3. How to Compile and Run

To compile the sample programs:
```bash
make
```

To run vector addition:
```bash
./vector <numThreads> <vectorSize>
# Example:
./vector 2 48000000
```

To run matrix multiplication:
```bash
./matrix <numThreads> <matrixSize>
# Example:
./matrix 2 1024
```

To clean build artifacts:
```bash
make clean
```

