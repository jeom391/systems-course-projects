# MLFQ Scheduler Simulation

## Overview

This project is a Multi-Level Feedback Queue (MLFQ) scheduler simulator implemented in C.
It uses three FIFO-based priority queues to model how processes move between different priority levels during CPU scheduling.

The simulator loads process data from a trace file, handles CPU execution and optional I/O blocking, and prints each process's turnaround time, response time, and the final simulation time.

## Features

- Three FIFO-based priority queues
- q1/q2 quantum = 10
- q3 quantum = 20
- Global priority boosting at fixed intervals
- File-based process input
- I/O-based voluntary CPU yielding
- Turnaround time and response time calculation
- Final simulation time output

## Input Format

Each line in a trace file has the following format:

```text
PID ArrivalTime RunTime IOStartTime IORuntime
```

Column meanings:

- `PID`: Process ID
- `ArrivalTime`: Time when the process enters the system
- `RunTime`: Total CPU time required by the process
- `IOStartTime`: CPU execution amount after which the process starts I/O
- `IORuntime`: Time spent performing I/O

If `IORuntime` is `0`, the process does not perform I/O.

Example:

```text
1 0 30 0 0
3 1 45 5 10
2 1 20 0 0
```

## Scheduling Policy

The scheduler has three priority queues: `q1`, `q2`, and `q3`.
All queues use FIFO ordering.

- The scheduler always selects a process from the highest non-empty priority queue.
- New processes enter `q1`.
- If a process uses all of its time quantum and has not completed, it is demoted.
- A process in `q1` is demoted to `q2`.
- A process in `q2` is demoted to `q3`.
- A process in `q3` is reinserted into `q3` after using its quantum.
- `q2` and `q3` are globally boosted to `q1` every 50 time units.
- When a process reaches its I/O start point, it voluntarily yields the CPU.
- After I/O completes, the process returns to the ready queue.

## How to Build and Run

Using `gcc`:

```bash
gcc src/MLFQ.c -o mlfq
./mlfq examples/trace1.txt
```

On Windows PowerShell, the executable may be run as:

```powershell
gcc src/MLFQ.c -o mlfq.exe
.\mlfq.exe examples\trace1.txt
```

Using Visual Studio:

1. Create or open a C/C++ console project.
2. Add `src/MLFQ.c` as the source file.
3. Build the project.
4. Run the executable with a trace file path, such as `examples\trace1.txt`.

## Sample Output

Example output from `output/trace_cpu_bound_result.txt`:

```text
PID	Turnaround Time	Response Time
1	55		0
2	40		10
3	25		17
Final time: 55
```

## What I Learned

This project helped me understand how MLFQ scheduling balances responsiveness, fairness, and starvation prevention.

One important fix was the priority boosting policy. Instead of treating boosting as an individual process waiting-time rule, the simulator now applies fixed-interval global boosting, where processes in lower-priority queues are promoted back to `q1` every 50 time units.
