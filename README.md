# Systems Course Projects

Selected course projects related to systems programming, compilers, and concurrency.

## Projects

### [SIC Assembler with SIC/XE Extension Plan](./sic-xe-assembler)
Implemented a two-pass SIC assembler in C, including symbol table management, literal handling, intermediate file generation, and object code generation.  
The project also documents a planned extension path toward SIC/XE features such as instruction formats, addressing modes, relocation records, and control sections.

### [miniC Compiler](./minic-compiler)
Implemented a lexical analyzer for a subset of the miniC language using C.

### [Lock Implementation and Analysis](./lock-implementation-analysis)
Implemented and analyzed synchronization mechanisms under concurrent execution scenarios.

### [MLFQ Scheduler Simulation](./mlfq-scheduler-simulation)

Implemented a Multi-Level Feedback Queue scheduler simulation in C.
The simulator manages three FIFO-based priority queues, each with its own time quantum, and supports priority boosting, process arrival handling, voluntary CPU yielding during I/O, and final scheduling metric calculation.

## Topics

- Systems Programming
- Assembly and Object Code Generation
- Compiler Construction
- Concurrency and Synchronization
- Low-level Debugging
