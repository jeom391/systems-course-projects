# miniC Compiler

## Overview

This project implements core components of a compiler for a miniC-like programming language.
The compiler processes source code through multiple stages such as lexical analysis, parsing, semantic analysis, and code generation or intermediate representation.

## Features

* Performs lexical analysis
* Parses miniC-like source code
* Handles grammar rules and syntax structures
* Performs semantic checks
* Reports syntax or semantic errors
* Generates intermediate representation or target output

## Project Structure

```text
minic-compiler/
├── README.md
├── src/
│   └── compiler source files
├── examples/
│   └── sample miniC input programs
└── tests/
    └── test cases
```

## How to Run

> This section will be updated after organizing the source code.

Example:

```bash
make
./minic examples/sample.mc
```

or

```bash
gcc -o minic src/*.c
./minic examples/sample.mc
```

## My Role

This was an individual course project.
I implemented the main compiler pipeline and debugged each stage of the compilation process.

## What I Learned

* How source code is processed through compiler stages
* How lexical analysis and parsing work together
* How grammar rules define valid program structures
* How semantic checks help detect invalid programs
* How compiler implementation connects theory with actual source code processing

## Keywords

`Compiler` `miniC` `Lexical Analysis` `Parsing` `Semantic Analysis`
