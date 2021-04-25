# BP Language

An LL(1) compiler written for EECE-5083: Compiler Theory and Practice class at University of Cincinnati. The professor did not give the language a name so I called it BP (Bootleg Pascal) due to similarities from my perspective. For this compiler, `.` was specifically made **required** as an indicator of EOF of the program.

## Requirements:
- clang >= 10.0.0 (project was built with clang 13.0.0)
- clang++ >= 10.0.0
- LLVM (13.0.0) - built from source and config in system path

## Project Layout:
- bin - executable of the compiler is generated to here
- dist - bitcode result of BP programs is generated to here
- obj - folder for storing obj files during linking process
- src - all necessary source codes.
- testPgms - test for correct and incorrect BP programs

## Usage
```
./bp.out <options> <file name>
Options:
  -dp     Show debug for parser
  -dt     Show debug symbol table
  -dm     Show in memory IR code from JIT
```

## Commands
Compile source codes to compiler

- `make`

This will create the compiler `bp.out` in the `bin` folder.

Run compiler to compile programs

- `bp.out <program name>.src`

Generate LLVM IR for debugging (run only after creating the executable)

- `make debug`

Run the program executable (can also use lli from LLVM manually)

- `make run`

## Special Thanks:
- uthash - A hashtable in C by Troy D. Hanson for symbol table implementation.

