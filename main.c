#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define GET_REG4(regs, i) (((regs) >> ((i) << 2)) & 0xF)
#define SET_REG4(regs, i, val)                     \
    ((regs) = ((regs) & ~(0xFULL << ((i) << 2))) | \
              (((uint64_t)(val) & 0xF) << ((i) << 2)))

#define GET_REG8(regs, i) (((regs) >> ((i) << 3)) & 0xFF)

#define SET_REG8(regs, i, val)                      \
    ((regs) = ((regs) & ~(0xFFULL << ((i) << 3))) | \
              (((uint64_t)(val) & 0xFF) << ((i) << 3)))

// ls: least significant nibble (bits 0-3)
// mid: middle nibble (bits 4-7)
// ms: most significant nibble (bits 8-11)
#define DEMUX_U12(ms, mid, ls) \
    ((uint16_t)(((ms) & 0xF) << 8) | (((mid) & 0xF) << 4) | ((ls) & 0xF))

#define FLAG_CARRY (1 << 0)
#define FLAG_TEST (1 << 1)

#define IS_SET(status, flag) (!!((status) & (flag)))

typedef struct {
    uint64_t registers;  // 16 4-bit words, sometimes addressed as pairs
    uint16_t pc;
    uint8_t accumulator;
    uint8_t status;
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

void cpu_tick(Intel4004* cpu)
{
    const uint8_t instruction = 0xca;
    const uint8_t opr = (instruction >> 4) & 0xf;
    const uint8_t opa = instruction & 0xf;

    switch (opr) {
        case 0x1: // JCN
            unimplemented(opr, opa);
        case 0x2: // FIM/SRC
            unimplemented(opr, opa);
        case 0x3: // FIN/JIN
            unimplemented(opr, opa);
        case 0x4: // JUN
            unimplemented(opr, opa);
        case 0x5: // JMX
            unimplemented(opr, opa);
        case 0x6: // INC
            unimplemented(opr, opa);
        case 0x7: // ISZ
            unimplemented(opr, opa);
        case 0x8: // ADD
            unimplemented(opr, opa);
        case 0x9: // SUB
            unimplemented(opr, opa);
        case 0xb: // LD/XCH
            unimplemented(opr, opa);
        case 0xc: // BBL
            unimplemented(opr, opa);
        case 0xd: // LDM
            unimplemented(opr, opa);
        case 0xe: // IO and RAM instructions
            unimplemented(opr, opa);
        case 0xf: // accumulator group instructions
            unimplemented(opr, opa);
        case 0x0: // NOP
        case 0xa: // undocumented
        default:
            cpu->pc += 1;
            break;
    }
}

alignas(64) const uint8_t program[] = {
#include "./firmware/busicom.inc"
};
const size_t programSz = sizeof(program);

int main(void)
{
    assert(programSz == 0x500);

    alignas(64) Intel4004 cpu;
    inspect_layout(&cpu);
}
