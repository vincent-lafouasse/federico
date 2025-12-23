#include <assert.h>
#include <stdint.h>
#include <sys/fcntl.h>
#include <unistd.h>

#define FIRMWARE_PATH "./firmware/busicom.bin"

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
