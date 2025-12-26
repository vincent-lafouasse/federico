#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define GET_REG4(regs, i) (((regs) >> ((i) << 2)) & 0xF)
#define SET_REG4(regs, i, val)                     \
    ((regs) = ((regs) & ~(0xFULL << ((i) << 2))) | \
              (((uint64_t)(val) & 0xF) << ((i) << 2)))

#define GET_REG8(regs, i) \
    ((GET_REG4(regs, 2 * i) << 4) | GET_REG4(regs, 2 * i + 1))

#define SET_REG8(regs, i, val)                   \
    do {                                         \
        SET_REG4(regs, 2 * i, (val >> 4) & 0xf); \
        SET_REG4(regs, 2 * i + 1, val & 0xf);    \
    } while (0)

// ls: least significant nibble (bits 0-3)
// mid: middle nibble (bits 4-7)
// ms: most significant nibble (bits 8-11)
#define DEMUX_U12(ms, mid, ls) \
    ((uint16_t)(((ms) & 0xF) << 8) | (((mid) & 0xF) << 4) | ((ls) & 0xF))

#define FLAG_CARRY (1 << 0)  // 1bit register
// pins to track:
#define FLAG_TEST (1 << 1)
// still  unsure if i need to track those pins
#define FLAG_CM_ROM (1 << 2)
#define FLAG_CM_RAM0 (1 << 3)
#define FLAG_CM_RAM1 (1 << 4)
#define FLAG_CM_RAM2 (1 << 5)
#define FLAG_CM_RAM3 (1 << 6)
// #define FLAG_UNUSED (1 << 7)

#define IS_SET(status, flag) (!!((status) & (flag)))
#define SET_FLAG(status, flag) ((status) |= (flag))
#define UNSET_FLAG(status, flag) ((status) &= ~(flag))
#define TOGGLE_FLAG(status, flag) ((status) ^= (flag))
#define SET_FLAG_BOOL(status, flag, truth) \
    ((truth) ? SET_FLAG(status, flag) : UNSET_FLAG(status, flag))

typedef struct {
    uint64_t registers;  // 16 4-bit words, sometimes as pairs eax/rax style
    uint16_t pc;
    uint8_t accumulator;
    uint8_t status;  // see FLAG_* macros
    uint16_t sp[3];  // maximum stack depth of 3
} Intel4004;

#define INSPECT_FIELD(base_ptr, field)                                      \
    do {                                                                    \
        ptrdiff_t offset = (char*)&(base_ptr->field) - (char*)base_ptr;     \
        size_t size = sizeof(base_ptr->field);                              \
        printf("%-12s | Offset: +%2td | Size: %zu bytes\n", #field, offset, \
               size);                                                       \
    } while (0)

void inspect_layout(Intel4004* cpu)
{
    printf("Base Address:    %p\n", cpu);
    printf("Size:            %zu bytes\n", sizeof(*cpu));
    printf("Aligned?:        %s\n", (uintptr_t)cpu % 64 == 0 ? "yes" : "no");

    printf("\nFields:\n");
    INSPECT_FIELD(cpu, registers);
    INSPECT_FIELD(cpu, pc);
    INSPECT_FIELD(cpu, accumulator);
    INSPECT_FIELD(cpu, status);
    INSPECT_FIELD(cpu, sp);
}

void unimplemented(uint8_t opr, uint8_t opa)
{
    printf("unimplemented: %02x %02x\n", opr, opa);
    exit(1);
}

uint8_t cpu_fetch(Intel4004* cpu, const uint8_t* program)
{
    const uint8_t instruction = program[cpu->pc & 0xfff];
    cpu->pc = (cpu->pc + 1) & 0xfff;
    return instruction;
}

bool jcn_condition(const Intel4004* cpu, uint8_t opa)
{
    const bool testCheck =
        ((opa >> 0) & 0x1) && !IS_SET(cpu->status, FLAG_TEST);
    const bool carryCheck =
        ((opa >> 1) & 0x1) && IS_SET(cpu->status, FLAG_CARRY);
    const bool accumulatorCheck = ((opa >> 2) & 0x1) && (cpu->accumulator == 0);
    const bool invert = (opa >> 3) & 0x1;

    const bool combined = accumulatorCheck || carryCheck || testCheck;
    return invert ? !combined : combined;
}

void cpu_tick(Intel4004* cpu, const uint8_t* program)
{
    const uint8_t instruction = cpu_fetch(cpu, program);
    const uint8_t opr = (instruction >> 4) & 0xf;
    const uint8_t opa = instruction & 0xf;

    switch (opr) {
        // 0b0001
        case 0x1: {  // JCN
            const uint8_t address = cpu_fetch(cpu, program);
            const bool cond = jcn_condition(cpu, opa);
            if (cond) {
                cpu->pc = cpu->pc & ~(0xff);
                cpu->pc = (cpu->pc | address) & 0xfff;
            }
            break;
        }
        // 0b0010
        case 0x2: {                                // FIM/SRC
            const uint8_t reg = (opa >> 1) & 0x3;  // RRR_

            if (opa & 0x1) {
                // SRC
                unimplemented(opr, opa);
            } else {
                // FIM: fetch imm., ROM value directly into regs
                const uint8_t value = cpu_fetch(cpu, program);
                SET_REG8(cpu->registers, reg, value);
            }
            break;
        }
        // 0b0011
        case 0x3:  // FIN/JIN
            unimplemented(opr, opa);
        // 0b0100
        case 0x4:  // JUN
            unimplemented(opr, opa);
        // 0b0101
        case 0x5:  // JMS
            unimplemented(opr, opa);
        // 0b0110
        case 0x6:  // INC
            // SET_REG4 already takes care of the overflow
            // it masks with 0xf before setting. no carry
            SET_REG4(cpu->registers, opa, GET_REG4(cpu->registers, opa) + 1);
            break;
        // 0b0111
        case 0x7:  // ISZ
            unimplemented(opr, opa);
        // 0b1000
        case 0x8:  // ADD
            cpu->accumulator +=
                GET_REG4(cpu->registers, opa) + IS_SET(cpu->status, FLAG_CARRY);
            SET_FLAG_BOOL(cpu->status, FLAG_CARRY, cpu->accumulator > 0xf);
            cpu->accumulator = cpu->accumulator & 0xf;
            break;
        // 0b1001
        case 0x9: {  // SUB
            cpu->accumulator += (~GET_REG4(cpu->registers, opa) & 0xf) +
                                IS_SET(cpu->status, FLAG_CARRY);
            SET_FLAG_BOOL(cpu->status, FLAG_CARRY, cpu->accumulator > 0xf);
            cpu->accumulator &= 0xf;
            break;
        }
        // 0b1010
        case 0xa:  // LD
            cpu->accumulator = GET_REG4(cpu->registers, opa);
            break;
        // 0b1011
        case 0xb: {  // XCH
            uint8_t acc = cpu->accumulator & 0xf;
            cpu->accumulator = GET_REG4(cpu->registers, opa);
            SET_REG4(cpu->registers, opa, acc);
            break;
        }
        // 0b1100
        case 0xc:  // BBL
            unimplemented(opr, opa);
        // 0b1101
        case 0xd:  // LDM
            cpu->accumulator = opa & 0xf;
            break;
        // 0b1110
        case 0xe:  // IO and RAM instructions
            unimplemented(opr, opa);
        // 0b1111
        case 0xf:  // accumulator group instructions
            unimplemented(opr, opa);
        case 0x0:  // NOP
        default:
            break;
    }
}

// max 16 4001 banks, each with a cap of 0x100
alignas(64) const uint8_t programROM[0x1000] = {
#include "./firmware/busicom.inc"
};

int main(void)
{
    alignas(64) Intel4004 cpu;
    inspect_layout(&cpu);
}
