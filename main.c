#include <assert.h>
#include <stdint.h>
#include <sys/fcntl.h>
#include <unistd.h>

#define FIRMWARE_PATH "./firmware/busicom.bin"

int main(void)
{
    int fd = open(FIRMWARE_PATH, O_RDONLY);
    uint8_t program[0x500];
    int sz = read(fd, program, 0x500);
    close(fd);

    assert(sz == 0x500);
}
