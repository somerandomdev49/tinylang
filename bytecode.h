#ifndef TINYLANG_BYTECODE_HEADER_
#define TINYLANG_BYTECODE_HEADER_

#define TINYLANG_CONST_TRUE 0xFFFF
#define TINYLANG_CONST_FALSE 0x0000

enum {
    reg_a,
    reg_x,
    reg_y,
    reg_z,
};

enum {
    flag_carry,
    flag_zero,
    flag_plus,
    flag_minus,
};

enum {
    ins_zer, // Zero
    ins_nop, // No-op

    // Memory operations
    ins_sta, // Store A in memory
    ins_lda, // Load A from memory
    ins_stx, // Store X in memory
    ins_ldx, // Load X from memory
    ins_sty, // Store Y in memory
    ins_ldy, // Load Y from memory
    ins_stz, // Store Z in memory
    ins_ldz, // Load Z from memory

    // Move operations
    ins_max, // Move A -> X
    ins_may, // Move A -> Y
    ins_maz, // Move A -> Z
    ins_mxa, // Move X -> A
    ins_mya, // Move Y -> A
    ins_mza, // Move Z -> A

    // Set operations
    ins_isa, // Immediate set A
    ins_isx, // Immediate set X
    ins_isy, // Immediate set Y
    ins_isz, // Immediate set Z

    // System operations
    ins_int, // Interrupt
    ins_ssp, // Set stack pointer (default: 0xE000);

    // Stack operations
    ins_pha, // Push A
    ins_phx, // Push X
    ins_phy, // Push Y
    ins_phz, // Push Z
    ins_pla, // Pull A
    ins_plx, // Pull X
    ins_ply, // Pull Y
    ins_plz, // Pull Z

    // Increment/Decrement operations
    ins_inc, // Increment A
    ins_inx, // Increment X
    ins_iny, // Increment Y
    ins_inz, // Increment Z
    ins_dec, // Decrement A
    ins_dex, // Decrement X
    ins_dey, // Decrement Y
    ins_dez, // Decrement Z

    // Mathematical/Logical operations
    ins_add, // Add to A
    ins_sub, // Substract from A
    ins_mul, // Multiply with A
    ins_div, // Divide A
    ins_and, // And with A
    ins_ora, // Or with A
    ins_xor, // X-Or with A
    ins_bit, // Bit test A
    ins_rsh, // Right-shift A
    ins_lsh, // Left-shift A
    ins_addi, // Add to A
    ins_subi, // Substract from A
    ins_muli, // Multiply with A
    ins_divi, // Divide A
    ins_andi, // And with A
    ins_orai, // Or with A
    ins_xori, // X-Or with A
    ins_biti, // Bit test A
    ins_rshi, // Right-shift A
    ins_lshi, // Left-shift
    ins_flag, // Bit-test FLAGS

    // Unary arithmetic/logic operations
    ins_neg, // Negate
    ins_not, // Binary not

    // Comparison operations
    ins_cmp, // Compare A
    ins_eqa, // Check A for equality
    ins_cpx, // Compare X
    ins_eqx, // Check X for equality
    ins_cpx, // Compare X
    ins_eqy, // Check Y for equality
    ins_cpy, // Compare Y
    ins_eqz, // Check Z for equality
    ins_cpz, // Compare Z
    ins_cmpi, // Compare A
    ins_eqai, // Check A for equality
    ins_cpxi, // Compare X
    ins_eqxi, // Check X for equality
    ins_cpxi, // Compare X
    ins_eqyi, // Check Y for equality
    ins_cpyi, // Compare Y
    ins_eqzi, // Check Z for equality
    ins_cpzi, // Compare Z

    // Control flow operations
    ins_jmp, // Unconditional jump
    ins_jnz, // Jump if not equal to zero
    ins_jez, // Jump if equal to zero
    ins_jeq, // Jump if equal
    ins_jne, // Jump if not equal
    ins_jgt, // Jump if greater than
    ins_jlt, // Jump if less than

    // Subroutine operations
    ins_ret, // Return from subroutine
    ins_cll, // Call subroutine
    ins_cla, // Call subroutine at an address in register
};

// 86 instructions.

#endif // TINYLANG_BYTECODE_HEADER_
