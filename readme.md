# Tiny Lang

> Tiny programming language with it's own register-based VM. WIP!

Compiler source: [main.c](main.c)
Virtual Machine source: [vm.c](vm.c)

## Language:

Single letter identifiers, no types (the only type is an integer),
no string literals, no character literals, no keywords (only key
symbols :]). Functions and variables are in separate namespaces.
No commas, you can write `xy` and that will be two separate tokens.

### Example:

As you can see, this language is very suitable for code golf,
although not designed for it.
```perl
@w(x){i=0;$(i<x){@D(1x);@P(1 32);@D(1i);@P(1 10)
;i=i+2;x=x+1};}@M(cv){a=3;b=2;c=a+b;@w(c);@(0);}
```

This is the "uncompressed" version (exactly the same):
```perl
@w(x) {
    i = 0;
    $(i < x) {
        @D(1 x);
        @P(1 32);
        @D(1 i);
        @P(1 10);
        i = i + 2;
        x = x + 1;
    };
}

@M(c v) {
    a = 3;
    b = 2;
    c = a + b;
    @w(c);
    @(0);
}
```

### Grammar:
TODO.

## Virtual Machine:

16-bit, has 8KB of memory, 4 registers (A, X, Y, Z).

### Memory:
```c
0x0000       : Always zero
0x0001-0x1000: Reserved
0x1001-0x2000: Stack
0x2001-0x9FFF: Data
0xA000-0xFFFF: Code
```

### Instruction Set:
see [bytecode.h](bytecode.h), TODO.

### Instruction Semantics:
see [bytecode.h](bytecode.h), TODO.

## Bytecode encoding:

Each instruction has it's own amount of operands required.
```
opcode operand
8-bit  16/8-bit
```
An operand is either an immediate 16-bit number or a
2-bit register number (8-bit with first 4 bits being 0).

 Decimal | Register | Operand Byte
:-------:|:--------:|:-----------:
    0    |     A    |  `00000000b`
    1    |     X    |  `00000001b`
    2    |     Y    |  `00000010b`
    3    |     Z    |  `00000011b`
