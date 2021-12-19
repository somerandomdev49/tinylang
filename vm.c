#include <stdio.h>
#include "bytecode.h"
#include "common.h"

void usage(char *pname) {
    printf("Usage:\n\t%s <file.bin>\n", pname);
}

typedef struct
{
    u16 regs[4];
    u16 p, s, f, r;
    u8 mem[0x10000];
} vm_t;
vm_t state;

u16 get_word_before(vm_t *state, u16 addr)
{
    return (state->mem[addr - 2] | (state->mem[addr - 1] << 8));
}

void run(vm_t *state)
{

#define W() (get_word_before(state, state->p += 2))
#define H() (state->mem[state->p++])
    for(;;)
    {
        // printf("Regs: A %.2d, X %.2d Y %.2d, Z %.2d, F %.2d, R %.2d, S %.2d, P %.2d\n",
        //     state->regs[0], state->regs[1], state->regs[2], state->regs[3],
        //     state->f, state->r, state->s, state->p);
        // printf("Stack: %.2d, %.2d\n", state->mem[4098], state->mem[4100]);
        u8 i = state->mem[state->p++];
        // printf("\033[0;32m%s\033[0;0m\n", ins_convert_to_string(i));
        switch(i) {
        case ins_hlt: return; // Halt
        case ins_nop: break; // No-op

#define O1(R) state->mem[W()] = state->regs[R]
#define O2(R) state->regs[R] = state->mem[W()]
        // Memory operations
        case ins_sta: O1(reg_a); break; // Store A in memory
        case ins_lda: O2(reg_a); break; // Load A from memory
        case ins_stx: O1(reg_x); break; // Store X in memory
        case ins_ldx: O2(reg_x); break; // Load X from memory
        case ins_sty: O1(reg_y); break; // Store Y in memory
        case ins_ldy: O2(reg_y); break; // Load Y from memory
        case ins_stz: O1(reg_z); break; // Store Z in memory
        case ins_ldz: O2(reg_z); break; // Load Z from memory
#undef O1
#undef O2
#define O1(SRC,DST) state->regs[DST] = state->regs[SRC]
        // Move operations
        case ins_max: O1(reg_a, reg_x); break; // Move A -> X
        case ins_may: O1(reg_a, reg_y); break; // Move A -> Y
        case ins_maz: O1(reg_a, reg_z); break; // Move A -> Z
        case ins_mxa: O1(reg_x, reg_a); break; // Move X -> A
        case ins_mya: O1(reg_y, reg_a); break; // Move Y -> A
        case ins_mza: O1(reg_z, reg_a); break; // Move Z -> A
#undef O1
#define O1(R) state->regs[R] = W()
        // Set operations
        case ins_isa: O1(reg_a); break; // Immediate set A
        case ins_isx: O1(reg_x); break; // Immediate set X
        case ins_isy: O1(reg_y); break; // Immediate set Y
        case ins_isz: O1(reg_z); break; // Immediate set Z
#undef O1
        // System operations
        case ins_int: /* todo */ break; // Interrupt
        case ins_ssp: state->s = W(); break; // Set stack pointer (default: 0x1000);

#define O1(R) state->mem[state->s] = state->regs[R]; state->s += 2
#define O2(R) state->regs[R] = state->mem[state->s -= 2];
        // Stack operations
        case ins_pha: O1(reg_a); break; // Push A
        case ins_phx: O1(reg_x); break; // Push X
        case ins_phy: O1(reg_y); break; // Push Y
        case ins_phz: O1(reg_z); break; // Push Z
        case ins_pla: O2(reg_a); break; // Pull A
        case ins_plx: O2(reg_x); break; // Pull X
        case ins_ply: O2(reg_y); break; // Pull Y
        case ins_plz: O2(reg_z); break; // Pull Z
#undef O1
#undef O2
#define O1(R) ++state->regs[R];
#define O2(R) --state->regs[R];
        // Increment/Decrement operations
        case ins_inc: O1(reg_a); break; // Increment A
        case ins_inx: O1(reg_x); break; // Increment X
        case ins_iny: O1(reg_y); break; // Increment Y
        case ins_inz: O1(reg_z); break; // Increment Z
        case ins_dec: O2(reg_a); break; // Decrement A
        case ins_dex: O2(reg_x); break; // Decrement X
        case ins_dey: O2(reg_y); break; // Decrement Y
        case ins_dez: O2(reg_z); break; // Decrement Z
#undef O1
#undef O2
#define O1(O) state->regs[0] = state->regs[0] O state->regs[H()]
#define O2(O) state->regs[0] = state->regs[0] O W()
        // Mathematical/Logical operations
        case ins_add: O1(+); break; // Add to A
        case ins_sub: O1(-); break; // Substract from A
        case ins_mul: O1(*); break; // Multiply with A
        case ins_div: O1(/); break; // Divide A
        case ins_and: O1(&); break; // And with A
        case ins_ora: O1(|); break; // Or with A
        case ins_xor: O1(^); break; // X-Or with A
        case ins_nxr: O1(==); break; // Not X-Or with A
        case ins_bit: /* todo */ break; // Bit test A
        case ins_rsh: O1(>>); break; // Right-shift A
        case ins_lsh: O1(<<); break; // Left-shift A
        case ins_addi: O2(+); break; // Add to A
        case ins_subi: O2(-); break; // Substract from A
        case ins_muli: O2(*); break; // Multiply with A
        case ins_divi: O2(/); break; // Divide A
        case ins_andi: O2(&); break; // And with A
        case ins_orai: O2(|); break; // Or with A
        case ins_xori: O2(^); break; // X-Or with A
        case ins_nxri: O2(==); break; // Not X-Or with A
        case ins_biti: /* todo */ break; // Bit test A
        case ins_rshi: O2(>>); break; // Right-shift A
        case ins_lshi: O2(<<); break; // Left-shift
        case ins_flag: /* todo */ break; // Bit-test FLAGS
#undef O1
#undef O2
#define O1(O) state->regs[0] = O state->regs[0]

        // Unary arithmetic/logic operations
        case ins_neg: O1(!); break; // Negate
        case ins_not: O1(~); break; // Binary not
#undef O1
#define O1(V) { /* tmp, for speed */ int x=(int)state->regs[0]-(int)V;state->f|=x>0?flag_plus:flag_minus;state->f|=x^0?0:flag_zero;}
        // Comparison operations
        case ins_cmp: O1(state->regs[H()]); break; // Compare A
        case ins_cpx: /* deprecated? */ break; // Compare X
        case ins_cpy: /* deprecated? */ break; // Compare Y
        case ins_cpz: /* deprecated? */ break; // Compare Z
        case ins_cmpi: O1(W()); break; // Compare A
        case ins_cpxi: /* deprecated? */ break; // Compare X
        case ins_cpyi: /* deprecated? */ break; // Compare Y
        case ins_cpzi: /* deprecated? */ break; // Compare Z

        // Control flow operations
        case ins_jmp: state->p = W(); break; // Unconditional jump
        case ins_jnz: /* todo */ break; // Jump if not equal to zero
        case ins_jez: /* todo */ break; // Jump if equal to zero
        case ins_jeq: /* todo */ break; // Jump if equal
        case ins_jne: /* todo */ break; // Jump if not equal
        case ins_jgt: /* todo */ break; // Jump if greater than
        case ins_jlt: /* todo */ break; // Jump if less than

        // Subroutine operations
        case ins_ret: // Return from subroutine
            state->p = state->mem[state->r];
            state->s = state->r - 2;
            state->r = state->mem[state->s];
            break;
        case ins_cll: // Call subroutine
            state->mem[state->s] = state->r;
            state->s += 2;
            state->mem[state->s] = state->p;
            state->s += 2;
            state->r = state->s - 2;
            state->p = W();
            break;
        case ins_cla: // Call subroutine at an address in register
            state->mem[state->s] = state->r;
            state->s += 2;
            state->mem[state->s] = state->p;
            state->s += 2;
            state->r = state->s - 2;
            state->p = state->regs[H()];
            break;
        default:
            fprintf(stderr, "Bad Instruction %d\n", state->mem[state->p - 1]);
            return;
        }
    }
}

int main(int argc, char *argv[]) {
    if(argc < 2) return usage(argv[0]), 1;

    FILE *f = fopen(argv[1], "rb");
    fseek(f, 0L, SEEK_END);
    long fsz = ftell(f);
    fseek(f, 0L, SEEK_SET);

    state.p = 0x2000;
    state.s = 0x1002;
    state.r = 0x1000;
    state.f = 0;

    fread(state.mem + 0x2000, 1, fsz > 0x8000 ? 0x8000 : fsz, f);
    fclose(f);

    run(&state);

    return state.regs[reg_a];
}
