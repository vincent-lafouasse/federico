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

typedef struct {
    uint64_t registers;
    uint16_t pc;
    uint16_t sp[3];
} Intel4004;

int main(void)
{
    int fd = open(FIRMWARE_PATH, O_RDONLY);
    uint8_t program[0x500];
    int sz = read(fd, program, 0x500);
    close(fd);

    assert(sz == 0x500);
}
