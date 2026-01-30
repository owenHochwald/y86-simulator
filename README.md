# Y86 Simulator

A Y86-64 instruction set simulator and validator written in C. This project implements a subset of the Y86-64 instruction set architecture, which is a simplified version of the x86-64 architecture.

## Features

- **Instruction Execution**: Supports core Y86 instructions including:
  - `halt` - Stop execution
  - `irmovq` - Immediate to register move
  - `rrmovq` - Register to register move
  - Arithmetic operations: `addq`, `subq`, `mulq`, `divq`, `modq`
  - Logical operations: `andq`, `xorq`

- **State Management**: Tracks complete machine state including:
  - 16 general-purpose registers
  - Program counter (PC)
  - Condition flags (Zero, Sign, Overflow)
  - Up to 1024 bytes of memory

- **Validation Framework**: Built-in test harness to verify simulator correctness

## Building

Compile the project using the provided Makefile:

```bash
make
```

This will create the `validator` executable along with the necessary object files.


## Memory Model

The simulator supports up to 1024 bytes of memory:
- `start_addr`: Base address of valid memory region
- `valid_mem`: Number of valid bytes starting from `start_addr`
- Memory addresses are checked for validity before access

## Registers

Y86 provides 15 addressable registers (indices 0-14), following the standard x86-64 naming:
- `%rax`, `%rcx`, `%rdx`, `%rbx`
- `%rsp`, `%rbp`, `%rsi`, `%rdi`
- `%r8` through `%r14`

## Condition Flags

The simulator maintains three condition flags:
- **Z (Zero)**: Set when result is zero
- **S (Sign)**: Set when result is negative
- **O (Overflow)**: Not actively maintained (per assignment specification)