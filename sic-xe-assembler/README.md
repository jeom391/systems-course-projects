# SIC Assembler

This project is a baseline SIC assembler implemented in C.

The original goal was a SIC/XE assembler, but the current stable implementation is intentionally limited to SIC. SIC/XE support is not implemented yet; the planned extension direction is documented in [docs/sic_xe_extension_plan.md](docs/sic_xe_extension_plan.md).

## Current Scope

Implemented:

- Two-pass assembler structure
- OPTAB loading and lookup
- SYMTAB creation and lookup
- LITTAB creation and lookup
- LOCCTR calculation
- START and END handling
- WORD, BYTE, RESW, and RESB directives
- Literal handling with LTORG and END literal pools
- Indexed addressing with `,X`
- Intermediate file generation
- SIC object file generation
- H/T/E record generation
- Text record flushing at 30 bytes and reservation boundaries

Not implemented:

- SIC/XE instruction formats 1, 2, 3, and 4
- `n/i/x/b/p/e` flag encoding
- Immediate and indirect addressing
- PC-relative and base-relative addressing
- BASE and NOBASE
- Modification records
- Control sections
- EXTDEF and EXTREF
- D/R/M records

## Project Structure

```text
.
|-- README.md
|-- Makefile
|-- asbr.vcxproj
|-- src/
|   |-- main.c
|   |-- assembler.h
|   |-- pass1.c
|   |-- pass2.c
|   |-- optab.c
|   |-- symtab.c
|   |-- littab.c
|   |-- parser.c
|   |-- object_writer.c
|   `-- utils.c
|-- examples/
|   |-- optab.txt
|   |-- srcfile.txt
|   |-- expected_intfile.txt
|   `-- expected_objfile.txt
|-- output/
`-- docs/
    `-- sic_xe_extension_plan.md
```

## Build

### Visual Studio

Open `asbr.vcxproj` in Visual Studio 2022 and build the `Debug|x64` configuration.

Command-line build from a Visual Studio Developer PowerShell or Developer Command Prompt:

```powershell
msbuild .\asbr.vcxproj /p:Configuration=Debug /p:Platform=x64
```

### GCC

If `gcc` and `make` are installed:

```sh
make
```

## Run

Visual Studio build:

```powershell
.\x64\Debug\asbr.exe
```

GCC build:

```sh
./sic_assembler
```

The default paths are defined in `src/assembler.h`:

- OPTAB: `examples/optab.txt`
- Source file: `examples/srcfile.txt`
- Intermediate file: `output/intfile.txt`
- Object file: `output/objfile.txt`

`src/main.c` keeps these defaults in one place so command-line arguments can be added later.

## Example Input

```asm
COPY    START   1000
FIRST   STL     RETADR
CLOOP   LDA     =C'EOF'
        STA     BUFFER,X
        LDCH    =X'05'
        STCH    BUFFER
        LTORG
        RSUB
RETADR  RESW    1
BUFFER  RESB    10
        END     FIRST
```

## Expected Object Output

```text
H COPY 001000 000023
T 001000 0F 14101600100F0C9019501012541019
T 00100F 07 454F46054C0000
E 001000
```

## Current Limitations

The assembler uses a simple line parser based on whitespace-separated fields. Error handling is also minimal: duplicate labels, undefined symbols, and undefined literals are reported, but assembly may continue. Invalid opcodes are not yet handled as first-class errors.

The intermediate file is still a flat text file. This is enough for the current SIC baseline, but it should be replaced with a structured intermediate representation before implementing SIC/XE.

## SIC/XE Extension Plan

SIC/XE should be added after the internal representation and object record generation are redesigned. See [docs/sic_xe_extension_plan.md](docs/sic_xe_extension_plan.md) for the recommended order.
