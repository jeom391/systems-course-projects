# Lock Implementation and Analysis

## Overview

This project implements and analyzes lock mechanisms used for synchronization in concurrent execution.
The goal of the project is to understand how mutual exclusion works and how different locking strategies affect correctness and performance.

## Features

* Implements lock-based synchronization
* Tests concurrent execution scenarios
* Analyzes correctness under shared-resource access
* Observes behavior under contention
* Compares synchronization behavior through experiment results

## Project Structure

```text
lock-implementation-analysis/
├── README.md
├── src/
│   └── source files
├── tests/
│   └── test programs
└── results/
    └── experiment results or analysis notes
```

## How to Run

> This section will be updated after organizing the source code.

Example:

```bash
gcc -pthread -o lock_test src/*.c
./lock_test
```

## My Role

This was an individual course project.
I implemented the synchronization logic, tested concurrent execution behavior, and analyzed the results.

## What I Learned

* Why race conditions occur in concurrent programs
* How locks enforce mutual exclusion
* How synchronization affects correctness and performance
* How contention changes program behavior
* How to reason about shared data in multi-threaded execution

## Keywords

`Operating Systems` `Concurrency` `Synchronization` `Locks` `Race Condition`
