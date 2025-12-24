#include <assert.h>
#include <stdint.h>
#include <sys/fcntl.h>
#include <unistd.h>

#define FIRMWARE_PATH "./firmware/busicom.bin"

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
#define FLAG_TEST  (1 << 1)

#define IS_SET(status, flag) (!!((status) & (flag)))

typedef struct {
    uint64_t registers; // 16 4-bit words, sometimes addressed as pairs
    uint16_t pc;
    uint16_t sp[3];     // maximum stack depth of 3
    uint8_t status;
} Intel4004;

int main(void)
{
    int fd = open(FIRMWARE_PATH, O_RDONLY);
    uint8_t program[0x500];
    int sz = read(fd, program, 0x500);
    close(fd);

    assert(sz == 0x500);
}
