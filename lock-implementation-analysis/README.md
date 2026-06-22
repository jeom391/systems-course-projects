# Lock Implementation and Analysis

## Overview

This project implements and analyzes several lock mechanisms used for mutual exclusion in concurrent programming.

The goal of this project is to understand how different lock implementations behave when multiple threads repeatedly access a shared counter. Each thread increments a shared variable inside a critical section, and the lock implementation ensures that only one thread can enter the critical section at a time.

The project compares lock behavior using performance metrics such as CPU usage, total runtime, lock waiting time, and the number of lock/unlock operations.

## Implemented Locks

This project includes three lock implementations:

| File | Lock Type | Description |
|---|---|---|
| `src/tas.c` | Test-and-Set Lock | A spin lock based on an atomic test-and-set operation. |
| `src/cas.c` | Compare-and-Swap Lock | A spin lock based on an atomic compare-and-swap operation. |
| `src/two.c` | Two-phase Lock | A hybrid lock that first spins using CAS and then moves to a sleep/wakeup phase. |

## Project Structure

```text
lock-implementation-analysis/
├── README.md
├── src/
│   ├── tas.c
│   ├── cas.c
│   └── two.c
└── docs/
    └── lock-analysis-report.pdf
```

## How to Run

Each source file can be compiled and executed independently.

```bash
gcc src/tas.c -o tas -pthread
./tas

gcc src/cas.c -o cas -pthread
./cas

gcc src/two.c -o two -pthread
./two
```

The number of threads and the number of iterations are configured using `#define` macros inside each source file.

Example:

```c
#define NUM_THREADS 8
#define ITERATIONS 1000000
```

To run experiments with different thread counts, modify the macro values and recompile the source file.

## Experiment Metrics

The following metrics are measured and printed by the programs:

- Total runtime
- CPU usage
- Lock count
- Unlock count
- Total lock waiting time
- Sleep count for the two-phase lock

These metrics are used to compare how each lock behaves as the number of threads increases.

## Implementation Summary

### Test-and-Set Lock

The TAS lock repeatedly calls a test-and-set operation until the lock becomes available.

If the lock is already held by another thread, the current thread continuously spins.

This implementation is simple, but it can waste a large amount of CPU time under high contention.

### Compare-and-Swap Lock

The CAS lock uses a compare-and-swap operation to acquire the lock only when the current lock value matches the expected value.

Although CAS avoids unnecessary writes when the lock is already held, it still behaves as a spin lock.

Therefore, under heavy contention, threads continue retrying and CPU usage can still become high.

### Two-phase Lock

The two-phase lock first tries to acquire the lock by spinning with CAS for a limited number of attempts.

If it fails, the thread enters a sleep state using a condition variable and is later awakened when the lock is released.

This approach reduces CPU waste compared to pure spin locks, especially when many threads compete for the same lock.

## Result Summary

As the number of threads increased, TAS and CAS showed rapidly increasing CPU usage because both implementations rely on busy waiting.

The two-phase lock showed lower CPU usage and lower total lock waiting time because threads that failed to acquire the lock could sleep instead of continuously spinning.

However, the two-phase lock is not always the best solution. Sleep and wakeup operations may involve scheduler overhead and context switching costs.

Therefore, lock performance should be evaluated by considering both CPU efficiency and runtime behavior.

## What I Learned

Through this project, I learned how low-level synchronization primitives such as test-and-set and compare-and-swap are used to implement mutual exclusion.

I also learned that lock performance cannot be judged only by runtime. A lock with a short runtime may still waste significant CPU resources, while a lock with better CPU efficiency may introduce scheduler-related overhead.

This project helped me understand the trade-off between simple spin locks and hybrid lock designs in operating systems.
