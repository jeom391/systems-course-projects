# SIC/XE Extension Plan

This document describes how to extend the current SIC assembler baseline toward a SIC/XE assembler without destabilizing the working SIC implementation.

## Why Not Add SIC/XE Directly?

The current code is a clean enough SIC baseline, but it is still strongly tied to SIC assumptions:

- Every instruction is treated as a 3-byte SIC instruction.
- Object code generation uses only `opcode + x bit + 15-bit address`.
- The intermediate file stores only `loc label opcode operand`.
- There is one global assembler context, one global literal table, and one flat object file flow.
- Text record flushing is tied directly to Pass2 instruction handling.

Adding SIC/XE behavior directly into this shape would make Pass2 very large and fragile. It would also make control sections, external references, relocation, and modification records difficult to implement correctly.

## Features To Add

SIC/XE support should include:

- Instruction formats 1, 2, 3, and 4
- `n/i/x/b/p/e` flag encoding
- Immediate addressing with `#`
- Indirect addressing with `@`
- Simple addressing
- PC-relative displacement calculation
- Base-relative displacement calculation
- BASE and NOBASE directives
- Extended format instructions using `+opcode`
- Modification record generation
- Control sections with CSECT
- EXTDEF and EXTREF handling
- D, R, and M record generation
- Literal pools scoped safely with control sections

## Required Structural Changes

### IntermediateLine

Replace the flat intermediate text representation with a structured record.

Suggested fields:

```c
typedef struct {
    int line_no;
    int loc;
    char section_name[FIELD_LEN];
    char label[FIELD_LEN];
    char opcode[FIELD_LEN];
    char operand[FIELD_LEN];
    int format;
    int object_length;
    int has_error;
} IntermediateLine;
```

The project may still write a readable intermediate file, but Pass2 should consume structured data or a richer serialized form.

### Instruction Encoder

Create a dedicated encoder for instructions:

- Parse `+`, `#`, `@`, and `,X`
- Determine instruction format
- Resolve symbol, literal, immediate, and numeric operands
- Select PC-relative, base-relative, or format 4 addressing
- Produce object code and relocation metadata

### Record Builder

Object record handling should be independent from Pass2.

The builder should support:

- H records
- T records
- E records
- M records
- D records
- R records
- Section-level flushing

### ControlSectionContext

The current `AssemblerContext` represents one whole program. SIC/XE needs section-level context.

Suggested fields:

```c
typedef struct {
    char name[FIELD_LEN];
    int start_address;
    int locctr;
    int length;
    int base_address;
    int has_base;
} ControlSectionContext;
```

Each control section should own or reference its own symbols, literals, external definitions, external references, and records.

### Error Model

Replace direct `printf("Error: ...")` calls with a structured error list.

This allows the assembler to:

- Report line numbers
- Keep multiple errors
- Stop object file generation when needed
- Keep Pass1 and Pass2 diagnostics consistent

## Recommended Development Order

1. Preserve the current SIC baseline with regression tests.
2. Introduce `IntermediateLine` while keeping current SIC output identical.
3. Split instruction encoding from Pass2.
4. Add format 1 and format 2 support.
5. Add format 3 flag parsing for simple, immediate, indirect, and indexed addressing.
6. Add PC-relative displacement.
7. Add BASE and NOBASE with base-relative displacement.
8. Add format 4 and modification records.
9. Add control section contexts.
10. Add EXTDEF, EXTREF, and D/R records.
11. Expand tests for multi-section programs and relocation.

## Testing Strategy

Keep the current SIC sample as the baseline regression test. Every SIC/XE change should still reproduce the same SIC `expected_intfile.txt` and `expected_objfile.txt` outputs for the existing example.

