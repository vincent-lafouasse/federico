#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
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
    uint16_t sp[3];  // maximum stack depth of 3
    uint8_t status;
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
    INSPECT_FIELD(cpu, sp);
    INSPECT_FIELD(cpu, status);
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
